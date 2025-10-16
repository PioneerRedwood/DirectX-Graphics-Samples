# MiniEngine Prototype Starter

이 폴더는 MiniEngine의 `Core` 모듈을 바로 활용해 작은 3D 연습 프로젝트를 구성할 수 있도록 최소한의 코드 골격과 실험 아이디어를 제공합니다.

## 구성 요소
- `PrototypeApp.cpp`: 기본 카메라, 렌더 루프, 리소스 전환 흐름을 담은 MiniEngine 애플리케이션 뼈대입니다.
- `Tasks.md`: 2D 그래픽스 프로그래머가 3D 엔진 구조를 익히면서 진행할 수 있는 작은 실험 과제를 정리했습니다.

## 빠른 시작
1. Visual Studio 개발자 명령 프롬프트를 열고 `MiniEngine` 폴더로 이동합니다.
2. `CreateNewSolution.bat PrototypeStarter`를 실행해 새 솔루션/프로젝트 골격을 생성합니다. (솔루션 이름은 원하는 이름으로 교체 가능)
3. 생성된 솔루션에서 `PrototypeApp.cpp`를 포함시키고 실행 대상으로 설정합니다.
4. 런타임에는 `Assets` 경로가 올바르게 설정되어야 하므로, 기존 `ModelViewer` 프로젝트와 동일한 작업 디렉터리를 사용하거나 `Renderer::Initialize()` 전에 `Model::SetAssetPath()`를 호출하세요.

## 확장 아이디어
- `Tasks.md`에 적힌 실험 순서대로 기능을 추가하며, Direct3D 12 명령 큐/리소스 배리어 흐름과 카메라 수학을 자연스럽게 익힐 수 있습니다.
- 렌더링 단계를 분리한 후 PIX 캡처를 통해 파이프라인 상태 변화를 관찰하면 이해가 빨라집니다.
