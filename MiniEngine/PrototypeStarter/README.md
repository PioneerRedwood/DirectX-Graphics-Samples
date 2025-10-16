# MiniEngine Prototype Starter

MiniEngine `Core` 모듈을 바로 활용해 간단한 3D 장면을 실험해 볼 수 있도록 구성한 최소 골격 프로젝트입니다.  
직접 렌더 루프를 편집하면서 MiniEngine의 파이프라인을 익히고, `Tasks.md`에 정리된 학습 과제를 단계적으로 수행해 보세요.

## 포함된 파일
- `PrototypeApp.cpp` – 기본 카메라 궤도 회전, 라인/삼각형 렌더링 등 샘플 장면을 구현한 진입점
- `PrototypeStarter.sln` / `PrototypeStarter.vcxproj` – Visual Studio 2022용 솔루션과 프로젝트
- `packages.config` – NuGet 의존성(WinPixEventRuntime, zlib)
- `Tasks.md` – MiniEngine 렌더링 흐름을 익히기 위한 권장 과제 모음

## 빌드 전 준비 사항
- Windows 10 19041 이상
- Visual Studio 2022(17.x) + **Desktop development with C++** 워크로드
  - MSVC v143 툴셋
  - Windows 10 SDK (10.0.19041 이상)
- GPU가 Direct3D 12를 지원해야 실행 시 화면을 확인할 수 있습니다.

## 빌드 방법
### Visual Studio IDE
1. `MiniEngine/PrototypeStarter/PrototypeStarter.sln`을 엽니다.
2. 솔루션 구성은 `Debug`(또는 원하는 구성), 플랫폼은 `Windows`를 선택합니다.
3. `F7` 또는 **Build > Build Solution**을 실행하면 `Core` 정적 라이브러리와 `PrototypeStarter` 실행 파일이 함께 빌드됩니다.

### 명령줄 (개발자 PowerShell/명령 프롬프트)
```powershell
# MiniEngine 루트에서 실행
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  MiniEngine/PrototypeStarter/PrototypeStarter.sln `
  /t:Restore,Build /p:Configuration=Debug /p:Platform=Windows
```
> 경로는 설치된 Visual Studio 에디션에 따라 다를 수 있습니다. `vswhere.exe -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe` 명령으로 정확한 msbuild 위치를 확인할 수 있습니다.

빌드 산출물은 `MiniEngine/Build/x64/<Configuration>/Output/PrototypeStarter/PrototypeStarter.exe`에 생성됩니다.

## 실행
1. 빌드가 완료되면 위 출력 폴더의 `PrototypeStarter.exe`를 실행합니다.
2. 창이 열리면 카메라가 삼각형과 XYZ 축을 중심으로 천천히 공전하는 장면을 확인할 수 있습니다.
3. GPU 드라이버나 Remote 환경 제약으로 실행이 어려울 경우 Visual Studio에서 **Debug > Start Without Debugging (Ctrl+F5)**으로 실행해 로그를 확인해 주세요.

## 다음 단계
- `Tasks.md`에 정리된 과제를 순서대로 진행하며 MiniEngine의 버퍼 관리, 루트 시그니처, 파이프라인 상태 객체 등을 직접 수정해 보세요.
- 더 큰 샘플이 필요하다면 `MiniEngine/ModelViewer/ModelViewer.sln`을 참고하여 에셋 로딩이나 포스트 프로세싱을 확인할 수 있습니다.
- 새 실험을 위한 프로젝트가 필요하면 `MiniEngine` 루트에서 `CreateNewSolution.bat <ProjectName>`을 실행해 템플릿을 생성한 뒤, 본 샘플과 동일한 방식으로 구성하면 됩니다.
