# Unreal UI Editing Guide

이 프로젝트는 C++ 게임 로직을 유지하면서 UI는 Unreal Editor의 UMG Widget Blueprint로 직접 수정할 수 있게 열어두었다.

## 1. Widget Blueprint 만들기

Content Browser에서 다음 순서로 만든다.

1. `Content/UI` 폴더 생성
2. `User Interface > Widget Blueprint` 생성
3. Parent Class를 `ShootUserWidget`으로 선택

추천 이름:

| 화면 | Widget Blueprint 이름 |
| --- | --- |
| 로비 | `WBP_Lobby` |
| 대시보드 | `WBP_Dashboard` |
| 캐릭터 선택 | `WBP_ShipSelect` |
| 게임 HUD | `WBP_Combat` |
| ESC 메뉴 | `WBP_PauseMenu` |
| 결과 화면 | `WBP_GameOver` |

## 2. HUD Blueprint 만들기

1. `Blueprint Class` 생성
2. Parent Class를 `ShootHUD`로 선택
3. 이름을 `BP_ShootHUD`로 지정
4. `BP_ShootHUD` Class Defaults에서 `Shoot > UMG` 카테고리에 각 Widget Blueprint를 넣는다.

예시:

- `Lobby Widget Class` = `WBP_Lobby`
- `Ship Select Widget Class` = `WBP_ShipSelect`
- `Combat Widget Class` = `WBP_Combat`
- `Pause Menu Widget Class` = `WBP_PauseMenu`

비워둔 항목은 기존 C++ Canvas UI가 fallback으로 표시된다.

## 3. GameMode에 HUD 연결

1. `Blueprint Class` 생성
2. Parent Class를 `ShootGameMode`로 선택
3. 이름을 `BP_ShootGameMode`로 지정
4. `HUD Class`를 `BP_ShootHUD`로 변경
5. Project Settings 또는 Map World Settings에서 GameMode를 `BP_ShootGameMode`로 지정

## 4. 버튼 기능 연결

Widget Blueprint에서 버튼을 선택한 뒤 `OnClicked` 이벤트에 다음 함수를 연결한다.

| 버튼 | 연결 함수 |
| --- | --- |
| Game Play | `OpenShipSelect` |
| Dashboard | `OpenDashboard` |
| Exit | `QuitGame` |
| Falcon 선택 | `SelectFalcon` |
| Titan 선택 | `SelectTitan` |
| Launch | `StartSelectedGame` |
| Back/Lobby | `OpenLobby` |
| Restart | `RestartGame` |
| ESC Resume | `ClosePauseMenu` |
| Volume + | `IncreaseMasterVolume` |
| Volume - | `DecreaseMasterVolume` |

## 5. HUD 값 표시

Text, Progress Bar 바인딩 또는 Event Tick에서 다음 값을 사용할 수 있다.

| 표시값 | 함수 |
| --- | --- |
| 플레이어 HP 비율 | `GetPlayerHealthRatio` |
| 플레이어 HP | `GetPlayerHealth` |
| 플레이어 Max HP | `GetPlayerMaxHealth` |
| 점수 | `GetPlayerScore` |
| 기체 이름 | `GetPlayerShipName` |
| 현재 Wave | `GetCurrentWave` |
| Wave 진행률 | `GetWaveProgressRatio` |
| 보스 HP 비율 | `GetBossHealthRatio` |
| 마스터 볼륨 | `GetMasterVolume` |
| 보스 활성 여부 | `IsBossActive` |
| 승리 여부 | `DidPlayerWin` |

## 6. 주의사항

- UI 배치, 색상, 버튼 추가/삭제는 Widget Blueprint 안에서 자유롭게 수정하면 된다.
- C++ Canvas UI는 fallback이다. Widget Class를 지정한 화면은 UMG가 우선 표시된다.
- 에디터에서 C++ 수정 후 적용이 안 되면 Live Coding을 끄거나 에디터를 종료한 뒤 `ShootCPPEditor` 빌드를 다시 해야 한다.
