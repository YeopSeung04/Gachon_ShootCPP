# Dual Ace: Sky Rift

언리얼 엔진 C++만으로 구현한 3D 플라이트 슈팅 게임입니다.  
중간고사 버전의 로비/선택/전투 흐름과 1인칭/3인칭 전환 아이디어를 유지하되, 기말 조건에 맞춰 핵심 플레이 로직을 모두 C++ 클래스에서 처리합니다.

## 프로젝트 콘셉트

플레이어는 실험용 전투기를 조종해 차원 균열에서 나타나는 적 드론 편대를 격추하고, 마지막 웨이브에서 등장하는 거대 지휘기를 파괴해야 합니다.

- 화면 흐름: Lobby -> Character Select -> Gameplay -> Result
- Lobby에는 Game Play, Dashboard, Exit 버튼이 있습니다.
- Dashboard에서는 로컬 리더보드 Top 5를 확인합니다.
- C++ Canvas 기반 캐릭터 선택 UI
- Falcon / Titan 2개 기체 선택
- 3D 공간에서 자동 전진하는 플라이트 슈팅
- `W / A / S / D` 기반 상하좌우 회피
- `Q / E`로 감속/가속
- `Left Mouse Button` 연사
- `V`로 콕핏 1인칭 시점과 추적 3인칭 시점 전환
- Wave가 진행될수록 적 속도, 체력, 등장 주기, 적 타입이 강화
- 최종 Wave에서 보스 등장
- 게임 종료 후 로컬 리더보드 Top 5 표시
- C++ HUD로 체력, 점수, 시점, Wave, 보스 체력 표시
- C++에서 엔진 내장 사운드를 로드해 발사/선택/보스/승패 사운드 재생

## 창의 요소

### 1. 2개 기체 선택

| 기체 | 특징 | 장점 | 단점 |
| --- | --- | --- | --- |
| Falcon | 고속 경량 전투기 | 빠른 이동, 빠른 연사 | 낮은 체력, 낮은 단발 피해 |
| Titan | 중장갑 전투기 | 높은 체력, 강한 탄환 | 느린 이동, 느린 연사 |

### 1. 시점 전환이 플레이 스타일을 바꿈

`V` 입력으로 단순 카메라 위치만 바꾸는 것이 아니라, 전투 감각이 달라지는 두 가지 모드를 제공합니다.

- 1인칭: 기체 전방에 붙은 콕핏 느낌의 빠른 조준 시점
- 3인칭: 넓은 회피 공간을 보기 좋은 추적 시점

### 2. Wave 기반 난이도 상승

Wave가 올라갈수록 적 스폰 간격이 짧아지고, 적 체력과 속도가 상승합니다.

- Wave 1: 기본 드론
- Wave 3 이후: 빠른 돌격 드론 추가
- Wave 4 이후: 체력 높은 방어 드론 추가
- Final Wave: 일반 적 제거 후 Command Carrier 보스 등장

### 3. 보스전 진입 구조

보스는 큰 충돌 범위와 높은 체력을 가지고 사인 곡선 패턴으로 움직여, 일반 적과 다른 전투 리듬을 만듭니다.

### 4. 로컬 리더보드

게임 종료 시 `USaveGame` 기반으로 점수, Wave, 기체, 승패, 생존 시간이 저장됩니다.  
결과 화면과 선택 화면에서 Top 5 기록을 확인할 수 있습니다.

### 5. 블루프린트 의존 최소화

기존 Blueprint 전용 구조와 다르게 아래 기능은 C++에서 직접 수행합니다.

- 캐릭터 선택 UI
- 플레이어 Pawn 생성 및 이동
- 총알 생성과 피해 처리
- 적/보스 체력 및 충돌 처리
- 적 Wave와 보스 스폰
- 게임 승리/패배 판정
- HUD 표시
- 입력 바인딩
- 리더보드 저장/로드
- 사운드 재생

## 조작법

| 입력 | 기능 |
| --- | --- |
| `Mouse Click` | Lobby / Dashboard / Character Select UI 버튼 클릭 |
| `1` | Falcon 선택 |
| `2` | Titan 선택 |
| `Enter` | 선택한 기체로 게임 시작 |
| `W / S` | 상승 / 하강 |
| `A / D` | 좌 / 우 이동 |
| `Q / E` | 감속 / 가속 |
| `Left Mouse Button` | 사격 |
| `V` | 1인칭 / 3인칭 시점 전환 |
| `R` | 게임 종료 후 재시작 |

## C++ 클래스 구조

```text
Source/ShootCPP/
  Public/
    CPlayer.h        플레이어 비행체, 사격, 시점 전환, 체력/점수
    Bullet.h         탄환 이동 및 적 피해 처리
    Enemy.h          일반 적 이동, 체력, 충돌 피해
    BossEnemy.h      보스 전용 체력/패턴
    ShootGameMode.h  선택 상태, Wave, 보스 스폰, 승패 판정
    ShootHUD.h       C++ Canvas HUD, 선택 화면, 리더보드
    ShootTypes.h     게임 상태, 기체 데이터, 리더보드 데이터
    LeaderboardSaveGame.h  로컬 기록 저장
  Private/
    CPlayer.cpp
    Bullet.cpp
    Enemy.cpp
    BossEnemy.cpp
    ShootGameMode.cpp
    ShootHUD.cpp
    LeaderboardSaveGame.cpp
```

## 제출 시 강조할 점

- 핵심 게임 로직은 Blueprint가 아니라 C++ 클래스에 구현되어 있습니다.
- 기본 메시 에셋은 언리얼 엔진 내장 BasicShapes를 C++에서 로드해 사용합니다.
- HUD도 UMG Blueprint가 아니라 C++ `AHUD::DrawHUD()`로 구현했습니다.
- 기존 중간고사 아이디어였던 1인칭/3인칭 전환을 기말 프로젝트에 맞게 전투 시스템과 연결했습니다.
- 캐릭터 선택, Wave 난이도, 보스전, 리더보드까지 게임의 시작/진행/종료 흐름을 모두 포함합니다.
