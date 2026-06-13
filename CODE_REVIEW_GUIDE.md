# ShootCPP 코드리뷰 설명서

이 문서는 기말 코드리뷰에서 "이 코드는 무엇을 위한 코드이고, 어떤 원리로 동작하는가"를 빠르게 설명하기 위한 자료다.

프로젝트는 Unreal Engine 5.7 기반 C++ 3D 슈팅 게임이다. 플레이어가 기체를 선택하고, 웨이브마다 등장하는 적을 처치하다가 마지막 보스를 잡으면 승리하는 구조다.

## 전체 구조

게임은 크게 네 부분으로 나뉜다.

1. 플레이어 전투 로직: 기체 이동, 발사, 체력, 궁극기, 카메라 전환
2. 게임 진행 로직: 메뉴 상태, 레벨 이동, 웨이브, 적 스폰, 점수, 리더보드 저장
3. 적/보스/투사체 로직: 일반 탄환, 궁극기 미사일, 일반 적 AI, 보스 패턴
4. UI/에디터 자동화: HUD, UMG 위젯 연결, DataTable/Map/UI 생성용 Commandlet

핵심 실행 흐름은 다음과 같다.

```text
게임 시작
-> AShootGameMode가 현재 맵을 보고 Lobby / ShipSelect / Playing 상태 결정
-> 플레이어가 기체 선택
-> GameplayMap으로 이동
-> StartGameplaySession()에서 플레이어 초기화, 1웨이브 시작
-> SpawnEnemy()가 타이머로 적 생성
-> 플레이어 Bullet/BigMissile로 적 처치
-> RegisterEnemyKilled()가 점수, 궁극기 충전, 콜아웃 처리
-> 마지막 웨이브에서 BossEnemy 생성
-> 보스 처치 시 EndGame(true), 사망 시 EndGame(false)
-> 결과를 SaveGame 리더보드에 저장
```

## Runtime 모듈

### Source/ShootCPP/Private/ShootCPP.cpp

이 파일은 게임 모듈을 Unreal Engine에 등록하기 위한 코드다.

`IMPLEMENT_PRIMARY_GAME_MODULE` 매크로가 `ShootCPP`를 기본 게임 모듈로 등록한다. 게임을 실행할 때 Unreal이 이 모듈을 로드하고, 그 안의 Actor, GameMode, HUD 같은 C++ 클래스를 사용할 수 있게 된다.

### Source/ShootCPP/Public/ShootCPP.h

프로젝트 공통 헤더다.

현재는 복잡한 로직을 담기보다 Unreal 프로젝트 기본 구조를 유지하는 역할에 가깝다. 다른 파일에서 공통 선언이 필요해질 때 이 헤더가 기준점이 될 수 있다.

### Source/ShootCPP/Public/ShootTypes.h

게임에서 공통으로 사용하는 데이터 타입을 정의한다.

`EShootGameState`는 현재 화면/게임 상태를 나타낸다. 예를 들어 `Lobby`, `ShipSelect`, `Playing`, `PauseMenu`, `GameOver` 상태가 있고, GameMode와 HUD가 이 값을 기준으로 입력 모드와 UI를 바꾼다.

`EPlayerShipType`은 선택 가능한 기체 타입이다. 현재는 `Falcon`과 `Titan`이 있다.

`FPlayerShipData`는 기체 능력치 데이터다. 속도, 체력, 탄환 피해량, 궁극기 피해량, 연사 간격, 색상을 담는다. 기체 선택 화면과 실제 전투 능력치가 같은 데이터를 사용한다.

`FEnemyStatData`는 적 능력치 데이터다. 이동 속도, 체력, 충돌 피해, 점수, 크기, 색상을 담는다.

`FLeaderboardEntry`는 게임 종료 후 저장되는 기록 한 줄이다. 점수, 도달 웨이브, 생존 시간, 승패 여부를 저장한다.

`FWaveDesign`은 웨이브 난이도 설계 데이터다. 스폰 간격, 한 번에 나오는 적 수, 빠른 적/탱크 적 확률, 보스 웨이브 여부를 담는다.

원리는 단순하다. 숫자와 상태값을 구조체로 묶어 두면, GameMode가 이 데이터를 읽어서 플레이어/적/웨이브를 같은 방식으로 초기화할 수 있다.

## 플레이어 코드

### Source/ShootCPP/Public/CPlayer.h
### Source/ShootCPP/Private/CPlayer.cpp

`ACPlayer`는 플레이어 기체를 담당하는 Pawn 클래스다.

이 코드는 다음 기능을 맡는다.

- 플레이어 기체 외형 생성
- 이동 입력 처리
- 일반 탄환 발사
- 궁극기 미사일 발사
- 체력과 피해 처리
- 벽 충돌 피해 처리
- 1인칭/3인칭 카메라 전환
- 기체 선택 데이터 적용

생성자에서는 `UBoxComponent`를 Root로 만들고, 그 아래에 여러 `UStaticMeshComponent`를 붙인다. 몸체, 날개, 조종석, 엔진을 Unreal 기본 도형으로 조합해서 기체 외형을 만든다.

`BeginPlay()`에서는 체력을 최대 체력으로 초기화하고, 동적 Material을 만들어 기체 색상과 궁극기 발광 효과를 바꿀 수 있게 준비한다.

`Tick()`은 매 프레임 실행된다. 현재 게임 상태가 `Playing`일 때만 입력값을 위치 변화로 바꾼다. 이동량은 `속도 * DeltaTime`으로 계산해서 FPS가 달라도 같은 속도로 움직이게 한다.

`SetupPlayerInputComponent()`는 Project Settings에 등록된 입력 이름과 C++ 함수를 연결한다. 예를 들어 `MoveRight`, `MoveForward`, `Roll`, `Ultimate`, `ToggleCamera` 같은 입력이 연결된다.

`ApplyShipData()`는 선택한 기체의 능력치를 실제 플레이어에 적용한다. Falcon이면 빠르고 체력이 낮으며, Titan이면 느리지만 체력과 공격력이 높다. 기체 크기와 색상도 여기서 바뀐다.

`Fire()`는 일반 공격 원리다. 발사 쿨타임이 0일 때 플레이어 앞쪽에 `ABullet`을 생성하고, 탄환에 피해량을 넘겨준다. 이후 쿨타임을 `_fireInterval`로 다시 설정해 연사 속도를 제한한다.

`UseUltimate()`는 궁극기 원리다. 궁극기 충전 수가 1 이상이면 하나를 소비하고, 플레이어 앞쪽에 `ABigMissile`을 생성한다. 이후 GameMode에 궁극기를 사용했다고 알려 웨이브별 충전 규칙을 갱신한다.

`ApplyDamage()`는 피해 처리 함수다. 체력을 깎고, 체력이 10% 이하가 되면 경고음을 재생한다. 체력이 0이 되면 GameMode의 `EndGame(false)`를 호출해 패배 처리한다.

`HandleArenaBounds()`는 이동 가능 영역 제한 함수다. 플레이어가 좌우/상하 경계를 넘으면 위치를 Clamp로 고정하고, 일정 간격으로 벽 충돌 피해를 준다.

`UpdateCamera()`는 카메라 전환 함수다. 같은 Pawn을 유지하면서 SpringArm 길이와 위치만 바꿔 콕핏 시점과 추적 시점을 전환한다.

## 투사체 코드

### Source/ShootCPP/Public/Bullet.h
### Source/ShootCPP/Private/Bullet.cpp

`ABullet`은 일반 탄환 Actor다.

역할은 플레이어가 발사한 탄환을 앞으로 이동시키고, 적과 충돌하면 피해를 주는 것이다.

생성자에서 충돌 박스와 원통형 Mesh를 만든다. `InitialLifeSpan = 3.0f`로 설정되어 있어, 적에게 맞지 않아도 3초 뒤 자동 삭제된다.

`Tick()`에서는 `GetActorForwardVector() * _speed * DeltaTime`만큼 이동한다. 즉 탄환은 자신의 앞 방향으로 계속 직진한다.

`OnHit()`은 Overlap 이벤트로 호출된다. 충돌한 Actor가 `AEnemy`이면 `Enemy->ApplyDamage(_damage)`를 호출하고, 일반 탄환은 한 번 맞으면 바로 `Destroy()`된다.

### Source/ShootCPP/Public/BigMissile.h
### Source/ShootCPP/Private/BigMissile.cpp

`ABigMissile`은 궁극기 미사일 Actor다.

일반 탄환보다 크고, 여러 적을 관통해서 맞출 수 있다. 핵심 차이는 `_damagedEnemies`라는 Set이다.

궁극기 미사일은 Overlap이 여러 번 발생할 수 있으므로, 이미 맞춘 적을 `_damagedEnemies`에 기록한다. 같은 적에게는 한 번만 피해를 주고, 다른 적은 계속 맞출 수 있다.

적을 처치했을 경우 미사일의 Owner가 플레이어인지 확인하고, 플레이어 최대 체력의 5%를 회복시킨다. 그래서 궁극기는 강한 공격이면서 회복 보상도 있는 기술이다.

## 적 코드

### Source/ShootCPP/Public/Enemy.h
### Source/ShootCPP/Private/Enemy.cpp

`AEnemy`는 일반 적의 공통 클래스다.

역할은 다음과 같다.

- 적 외형 생성
- 플레이어 추적 이동
- 체력/피해 처리
- 플레이어와 충돌 시 피해
- 죽었을 때 GameMode에 점수 처리 요청

생성자에서는 플레이어처럼 기본 도형을 조합해 적 외형을 만든다. 몸체, 코어, 좌우 날개를 Mesh Component로 구성한다.

`Tick()`에서는 플레이어 위치를 가져와 X/Y 평면에서 플레이어 방향으로 이동한다. Z는 플레이어 높이에 맞춰 2.5D 슈팅처럼 움직인다. 플레이어보다 너무 뒤로 지나간 적은 삭제해서 Actor가 계속 쌓이지 않게 한다.

`InitializeEnemy()`는 GameMode가 적을 Spawn한 뒤 능력치를 넣어주는 함수다. Basic, Fast, Tank, Boss 데이터에 따라 속도, 체력, 충돌 피해, 점수, 크기, 색상이 바뀐다.

`ApplyDamage()`는 탄환이나 미사일이 호출한다. 체력을 깎고, 체력이 0 이하가 되면 `DestroyByPlayer()`를 호출한다.

`DestroyByPlayer()`는 GameMode의 `RegisterEnemyKilled()`를 호출한다. 점수 증가, 킬 콜아웃, 궁극기 충전은 적이 직접 하지 않고 GameMode가 처리한다.

`OnHit()`은 플레이어와 충돌했을 때 호출된다. 플레이어에게 `_contactDamage`만큼 피해를 주고, 일반 적은 사라진다. 보스는 `_isBoss`가 true라서 충돌 후에도 사라지지 않는다.

### Source/ShootCPP/Public/BossEnemy.h
### Source/ShootCPP/Private/BossEnemy.cpp

`ABossEnemy`는 `AEnemy`를 상속한 보스 클래스다.

보스는 일반 적의 체력, 피해, 점수, 충돌 처리를 그대로 사용하고, 추가로 보스 외형과 공격 패턴을 가진다.

보스 패턴은 상태 머신으로 동작한다.

```text
Tracking
-> Charging
-> Dashing
-> Retreating
-> Tracking 반복
```

`Tracking` 상태에서는 플레이어 앞쪽을 따라다닌다.

`Charging` 상태에서는 돌진 전 예고처럼 속도를 늦추고 코어 발광을 키운다.

`Dashing` 상태에서는 플레이어 뒤쪽을 목표로 빠르게 돌진한다.

`Retreating` 상태에서는 다시 플레이어 앞쪽 위치로 복귀해 다음 패턴을 준비한다.

`UpdateChargeVisual()`은 돌진 예고 효과를 만든다. ChargeRatio가 커질수록 코어 Mesh가 커지고, 색상과 Emissive 값이 바뀐다. 플레이어가 보스 돌진 타이밍을 시각적으로 알 수 있게 하는 코드다.

## 게임 진행 코드

### Source/ShootCPP/Public/ShootGameMode.h
### Source/ShootCPP/Private/ShootGameMode.cpp

`AShootGameMode`는 게임 전체의 중앙 관리자다.

담당하는 기능은 다음과 같다.

- 기본 Pawn, HUD, PlayerController 지정
- 현재 게임 상태 관리
- 로비, 대시보드, 기체 선택, 전투, 일시정지, 게임오버 전환
- 맵 이동
- 기체 데이터, 적 데이터, 웨이브 데이터 로드
- 적 스폰과 웨이브 진행
- 보스 소환
- 점수와 궁극기 충전 관리
- 사운드 큐 관리
- 리더보드 저장/로드
- 입력 모드 변경

생성자에서는 `DefaultPawnClass`, `HUDClass`, `PlayerControllerClass`를 지정한다. 이 설정 때문에 레벨이 시작되면 `ACPlayer`, `AShootHUD`, `AShootPlayerController`가 기본으로 사용된다.

또한 C++ 기본 밸런스 데이터를 먼저 만든 뒤 DataTable을 로드한다. DataTable이 있으면 테이블 값으로 덮어쓰고, 없으면 C++ 기본값으로도 게임이 실행된다.

`BeginPlay()`에서는 SoundMix, AudioComponent, 리더보드, 선택 기체, 현재 맵 상태를 초기화한다. 현재 맵 이름이 `LobbyMap`, `CharacterSelectMap`, `GameplayMap` 중 무엇인지 보고 초기 게임 상태를 정한다.

`OpenLobby()`, `OpenDashboard()`, `OpenShipSelect()`, `StartSelectedGame()`은 메뉴 흐름을 담당한다. 현재 맵이 다르면 `QueueLevelTransition()`으로 맵을 이동하고, 같은 맵이면 상태만 바꾼다.

`StartGameplaySession()`은 전투 시작 함수다. 게임 상태를 `Playing`으로 바꾸고, 플레이어 위치/회전/체력/점수/입력을 초기화한 뒤 `StartWave()`를 호출한다.

`StartWave()`는 현재 웨이브를 시작한다. 스폰 타이머를 걸어서 `SpawnEnemy()`가 반복 실행되게 하고, 웨이브 시간이 끝나면 `AdvanceWave()`가 실행되게 한다.

`SpawnEnemy()`는 웨이브 데이터에 따라 적을 생성한다. `FWaveDesign`의 `TankEnemyChance`, `FastEnemyChance`를 이용해 Tank, Fast, Basic 중 하나를 선택한다. 그리고 웨이브별 속도/체력/피해 배율을 적용한다.

`AdvanceWave()`는 다음 웨이브로 넘어간다. 다음 웨이브가 보스 웨이브이면 `SpawnBoss()`를 호출하고, 아니면 다시 `StartWave()`를 호출한다.

`SpawnBoss()`는 보스전 시작 함수다. 기존 일반 적과 타이머를 정리하고, 플레이어에게 궁극기 2회를 지급한 뒤 `ABossEnemy`를 소환한다.

`RegisterEnemyKilled()`는 적 처치 결과가 모이는 함수다. 플레이어 점수를 증가시키고, 킬 콜아웃을 재생하고, 조건이 맞으면 궁극기를 충전한다. 보스를 처치한 경우 `EndGame(true)`로 승리 처리한다.

`EndGame()`은 게임 종료 함수다. 스폰 타이머와 웨이브 타이머를 정리하고, 승패 여부를 저장하고, 리더보드에 결과를 추가한다.

`PlayVoiceSound()`, `PlayImportantVoiceSound()`, `PlayCalloutSound()`는 사운드 큐를 관리한다. 여러 음성이 동시에 겹치지 않게 배열에 넣고 순서대로 재생한다. 궁극기 준비처럼 중요한 소리는 큐 앞에 넣는다.

`LoadLeaderboard()`, `SaveLeaderboardEntry()`, `SortAndTrimLeaderboard()`는 SaveGame 기반 리더보드 시스템이다. 게임 종료 시 기록을 저장하고, 점수 높은 순으로 정렬한 뒤 Top 5만 유지한다.

## 입력 코드

### Source/ShootCPP/Public/ShootPlayerController.h
### Source/ShootCPP/Private/ShootPlayerController.cpp

`AShootPlayerController`는 키보드와 마우스 입력을 해석하는 클래스다.

원리는 Controller가 입력을 직접 계산해서 Player와 GameMode에 전달하는 방식이다. 이동/발사는 Player에게 보내고, 메뉴 선택/게임 시작/일시정지는 GameMode에게 보낸다.

`SetupInputComponent()`는 `Fire`, `SelectFalcon`, `SelectTitan`, `ConfirmStart`, `RestartGame`, `PauseMenu`, `MoveRight`, `MoveForward`, `Roll` 입력을 함수에 연결한다.

`HandlePrimaryPressed()`는 좌클릭 처리다. 전투 중이면 플레이어의 `StartFire()`를 호출하고, 메뉴 상태이면 마우스 좌표를 GameMode의 `HandlePrimaryClick()`에 넘긴다.

`HandleRestart()`는 상태에 따라 다르게 동작한다. 게임오버 상태이면 재시작하고, 전투 중이면 궁극기 사용으로 처리한다.

`PollMovementKeys()`는 매 프레임 WASD/QE 키 상태를 직접 확인한다. Axis 이벤트가 누락되거나 메뉴 전환 중 입력이 남는 상황을 방지하기 위해, 전투 상태가 아니면 입력값을 0으로 초기화한다.

`ApplyMenuInputMode()`, `ApplyGameInputMode()`, `ApplyPauseInputMode()`는 마우스 커서와 입력 모드를 상태별로 바꾼다.

## UI 코드

### Source/ShootCPP/Public/ShootHUD.h
### Source/ShootCPP/Private/ShootHUD.cpp

`AShootHUD`는 화면 UI를 그리는 클래스다.

이 프로젝트는 두 가지 UI 방식을 같이 지원한다.

1. UMG 위젯이 있으면 해당 위젯을 화면에 올린다.
2. UMG가 없으면 Canvas 기반 fallback UI를 직접 그린다.

`DrawHUD()`는 매 프레임 호출된다. 먼저 현재 GameMode 상태를 확인하고, 해당 상태에 맞는 UMG 위젯이 있으면 `SyncWidgetForState()`로 위젯을 화면에 올린다.

UMG를 사용 중이어도 적 체력바와 궁극기 상태 같은 오버레이는 HUD가 직접 그린다. 적 체력바는 3D 월드 좌표를 화면 좌표로 변환해서 적 머리 위에 표시한다.

UMG 위젯이 없으면 `DrawLobby()`, `DrawDashboard()`, `DrawShipSelect()`, `DrawCombatHud()`, `DrawPauseMenu()`, `DrawGameOver()` 함수가 Canvas로 직접 UI를 그린다.

`DrawButton()`은 Canvas 버튼을 그리는 함수다. 실제 UMG Button이 아니므로 `AddHitBox()`로 클릭 가능한 사각형 영역을 등록한다.

`NotifyHitBoxClick()`은 Canvas HitBox 클릭을 받는다. 클릭된 BoxName에 따라 GameMode의 메뉴 함수들을 호출한다.

### Source/ShootCPP/Public/ShootUserWidget.h
### Source/ShootCPP/Private/ShootUserWidget.cpp

`UShootUserWidget`은 UMG Blueprint와 C++ 게임 로직을 연결하는 클래스다.

UMG Designer에서 만든 버튼 이름과 C++ 함수를 연결한다. 예를 들어 `Lobby_PlayButton`은 `OpenShipSelect()`에, `GameOver_RestartButton`은 `RestartGame()`에 연결된다.

`NativeConstruct()`에서 버튼 바인딩을 수행한다. `GetWidgetFromName()`으로 위젯을 찾고, 존재하면 클릭 이벤트를 C++ 함수에 연결한다. 위젯이 없으면 건너뛰기 때문에 일부 UI만 있어도 크래시가 나지 않는다.

`NativeTick()`에서는 매 프레임 `UpdateRuntimeText()`를 호출한다. 체력, 점수, 웨이브, 보스 체력, 볼륨, 최종 결과처럼 계속 바뀌는 값을 UMG 텍스트와 ProgressBar에 반영한다.

`GetPlayerHealthRatio()`, `GetPlayerScore()`, `GetCurrentWave()` 같은 함수들은 Blueprint에서도 호출할 수 있는 데이터 조회 함수다.

`SetTextByName()`, `SetProgressByName()`, `SetVisibilityByName()`은 위젯 이름으로 UI 요소를 찾아 값을 바꾸는 유틸리티 함수다.

## 배경/아레나 코드

### Source/ShootCPP/Public/SpaceArena.h
### Source/ShootCPP/Private/SpaceArena.cpp

`ASpaceArena`는 우주 배경과 통로 장식을 무한히 이어지는 것처럼 보이게 하는 Actor다.

맵에 미리 배치된 바닥, 레일, 게이트, 별 Actor를 태그로 찾는다. 예를 들어 `ArenaFloorSegment`, `ArenaLeftRail`, `ArenaStar` 같은 태그를 가진 Actor를 배열에 저장한다.

`BeginPlay()`에서는 태그로 Actor를 모으고, 각 Actor의 Material 색상을 코드로 설정한다.

`Tick()`에서는 플레이어의 X 위치를 기준으로 장식 Actor를 재활용한다. 플레이어 뒤로 너무 멀어진 바닥 조각은 전체 길이만큼 앞으로 이동시킨다. 그래서 실제로는 제한된 개수의 Actor만 있지만, 플레이어 입장에서는 무한히 이어지는 배경처럼 보인다.

## 저장 코드

### Source/ShootCPP/Public/LeaderboardSaveGame.h
### Source/ShootCPP/Private/LeaderboardSaveGame.cpp

`ULeaderboardSaveGame`은 리더보드 저장용 SaveGame 클래스다.

`Entries` 배열 하나를 가지고 있고, 여기에 `FLeaderboardEntry`들이 저장된다. 실제 저장/로드 로직은 GameMode의 `LoadLeaderboard()`와 `SaveLeaderboardEntry()`에서 처리한다.

`.cpp` 파일은 현재 별도 로직 없이 헤더 구현을 위한 빈 파일이다.

### Source/ShootCPP/Public/ShootGameInstance.h
### Source/ShootCPP/Private/ShootGameInstance.cpp

`UShootGameInstance`는 맵이 바뀌어도 유지되어야 하는 임시 데이터를 저장한다.

GameMode는 레벨이 바뀔 때 새로 만들어지지만, GameInstance는 게임 실행 중 유지된다. 그래서 기체 선택 화면에서 고른 `Falcon` 또는 `Titan` 값을 GameInstance에 저장하고, GameplayMap으로 이동한 뒤 다시 읽어온다.

`SetSelectedShipType()`은 선택한 기체를 저장하고, `GetSelectedShipType()`은 저장된 기체를 반환한다.

## Editor 모듈

Editor 모듈은 실제 게임 플레이 중에는 실행되지 않는다. Unreal Editor에서 에셋을 자동 생성하거나 수정하기 위한 도구 코드다.

### Source/ShootCPPEditor/Private/ShootCPPEditorModule.cpp

`ShootCPPEditor` 모듈을 Unreal Editor에 등록한다.

`IMPLEMENT_MODULE` 매크로를 통해 Editor 전용 모듈을 로드할 수 있게 한다. 이 모듈 안에 Commandlet들이 포함된다.

### Source/ShootCPPEditor/Public/CreateShootDataTablesCommandlet.h
### Source/ShootCPPEditor/Private/CreateShootDataTablesCommandlet.cpp

`UCreateShootDataTablesCommandlet`은 게임 데이터 테이블을 자동 생성하는 에디터 도구다.

생성하는 DataTable은 다음과 같다.

- `/Game/Data/DT_PlayerStats`
- `/Game/Data/DT_EnemyStats`
- `/Game/Data/DT_WaveDesigns`

`CreateOrResetDataTable()`은 지정한 경로에 DataTable을 만들거나 기존 테이블을 초기화한다.

`PopulatePlayerStats()`는 Falcon과 Titan의 능력치를 넣는다.

`PopulateEnemyStats()`는 Basic, Fast, Tank, Boss 적 데이터를 넣는다.

`PopulateWaveDesigns()`는 1~5웨이브 설계 데이터를 넣는다.

`Main()`은 위 세 함수를 실행하고, 변경된 Package들을 저장한다.

### Source/ShootCPPEditor/Public/CreateShootMapsCommandlet.h
### Source/ShootCPPEditor/Private/CreateShootMapsCommandlet.cpp

`UCreateShootMapsCommandlet`은 게임 맵을 자동 준비하는 에디터 도구다.

기준 맵인 `/Game/Maps/ShootingMap`을 복사해서 다음 맵을 만든다.

- `/Game/Maps/LobbyMap`
- `/Game/Maps/CharacterSelectMap`
- `/Game/Maps/GameplayMap`

`CopySourceMapIfNeeded()`는 대상 맵 파일이 없으면 원본 맵을 복사한다.

`ClearGeneratedActors()`는 이전에 자동 생성된 Actor를 삭제한다. `GeneratedArena` 태그가 붙은 Actor만 지우기 때문에, 다른 수동 배치 Actor는 건드리지 않는다.

`AddArenaGeometry()`는 바닥, 좌우 레일, 게이트, 별, `ASpaceArena` 관리 Actor를 배치한다. 각 Actor에는 SpaceArena가 찾을 수 있는 태그가 붙는다.

`PopulateMap()`은 맵을 로드하고, 기존 생성 Actor를 지운 뒤, 새 아레나 장식을 만들고 저장한다.

### Source/ShootCPPEditor/Public/PopulateShootUICommandlet.h
### Source/ShootCPPEditor/Private/PopulateShootUICommandlet.cpp

`UPopulateShootUICommandlet`은 UMG 위젯 Blueprint의 기본 레이아웃을 자동으로 채우는 도구다.

대상 위젯은 다음과 같다.

- `WBP_Lobby`
- `WBP_Dashboard`
- `WBP_ShipSelect`
- `WBP_Combat`
- `WBP_PauseMenu`
- `WBP_GameOver`

`LoadWidgetBlueprint()`는 지정 경로의 Widget Blueprint를 불러온다.

`ResetRoot()`는 기존 Root Widget을 제거하고 새 `CanvasPanel`을 Root로 설정한다.

`AddText()`, `AddButton()`, `AddProgress()`는 TextBlock, Button, ProgressBar를 생성해서 Canvas에 배치한다.

중요한 원리는 위젯 이름이다. 예를 들어 `Lobby_PlayButton`, `Combat_HealthText`, `Pause_VolumeBar` 같은 이름으로 위젯을 만들면, `UShootUserWidget`의 C++ 코드가 그 이름을 찾아 값을 갱신하거나 클릭 이벤트를 연결한다.

## Build 설정 코드

### Source/ShootCPP/ShootCPP.Build.cs

`ShootCPP` 런타임 모듈의 의존성을 설정한다.

`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `UMG`를 Public Dependency로 사용한다. UI 구현을 위해 `Slate`, `SlateCore`도 Private Dependency로 추가되어 있다.

### Source/ShootCPPEditor/ShootCPPEditor.Build.cs

`ShootCPPEditor` 에디터 모듈의 의존성을 설정한다.

Editor에서 DataTable, Widget Blueprint, Map 등을 수정해야 하므로 `UnrealEd`, `Kismet`, `UMGEditor`, `AssetRegistry`, 그리고 런타임 모듈인 `ShootCPP`에 의존한다.

### Source/ShootCPP.Target.cs

게임 실행/패키징용 Target 설정이다.

`Type = TargetType.Game`이라 실제 게임 빌드에 사용된다. `ExtraModuleNames.Add("ShootCPP")`로 런타임 모듈을 포함한다.

### Source/ShootCPPEditor.Target.cs

Unreal Editor에서 프로젝트를 열 때 사용하는 Target 설정이다.

`Type = TargetType.Editor`라 에디터 빌드에 사용된다. 런타임 모듈 `ShootCPP`가 포함되고, `.uproject`에 등록된 Editor 모듈도 함께 로드될 수 있다.

## 코드리뷰 때 말할 핵심 포인트

이 게임의 중심은 `AShootGameMode`다. GameMode가 현재 상태, 웨이브, 적 스폰, 점수, 저장, 사운드 큐를 관리한다.

플레이어인 `ACPlayer`는 GameMode가 허락한 `Playing` 상태에서만 움직이고 공격한다. 그래서 메뉴 상태와 전투 상태가 섞이지 않는다.

적은 `AEnemy` 공통 클래스로 만들고, 보스는 `ABossEnemy`가 이를 상속해 패턴만 확장했다. 상속을 사용해 중복을 줄인 구조다.

데이터는 `ShootTypes.h`의 구조체와 DataTable을 통해 관리한다. C++ 기본값이 있고, DataTable이 있으면 그 값으로 덮어써서 에디터에서 밸런스를 바꾸기 쉽다.

UI는 UMG가 있으면 UMG를 사용하고, 없으면 Canvas HUD가 fallback으로 동작한다. 덕분에 UI 에셋이 덜 준비되어도 게임 확인이 가능하다.

Editor Commandlet은 반복 작업을 자동화하기 위한 코드다. DataTable, 맵, UMG 기본 배치를 코드로 만들 수 있어서 결과물을 재생성하기 쉽다.

## 검증 내용

설명 주석과 이 문서 작성 후 Unreal Engine 5.7 기준으로 에디터 빌드를 확인했다.

```bash
/Users/Shared/Epic\ Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh ShootCPPEditor Mac Development -Project="/Users/han-seung-yeop/Documents/GitHub/Gachon_ShootCPP/ShootCPP.uproject" -WaitMutex
```

결과는 `Result: Succeeded`였다.
