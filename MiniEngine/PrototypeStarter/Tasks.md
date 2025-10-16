# Prototype 학습 과제

MiniEngine이 제공하는 시스템을 따라가며 2D 렌더러 관점에서 3D 파이프라인을 익히기 위한 제안 단계입니다.

1. **카메라와 버퍼 구조 살펴보기**  
   - `Core/Camera.h`와 `Core/BufferManager.cpp`의 전역 버퍼 생성 방식을 확인하고, `PrototypeApp.cpp`에 포함된 카메라 파라미터를 조정해 보세요.
2. **색상/깊이 버퍼 전환 실험**  
   - `GraphicsContext::TransitionResource()` 호출을 주석 처리해보고 PIX로 결과를 확인하여 배리어의 역할을 체감합니다.
3. **간단한 기하 추가**  
   - `GraphicsContext::SetDynamicVertexBuffer()`와 `Draw()` 호출로 오른손 좌표계 기준의 축/삼각형을 그려 보고, 월드-뷰-프로젝션 행렬 구성을 실습합니다.
4. **상수 버퍼 업로드**  
   - `UploadBuffer`를 사용해 MVP 행렬을 셰이더에 전달하고, 카메라 이동 시 화면 변화를 확인합니다.
5. **깊이 테스트 비교**  
   - `GraphicsContext::GetDefaultDepthStencil()`을 적용한 PSO를 만들어 깊이 테스트 on/off 차이를 시험합니다.
6. **후처리 맛보기**  
   - `PostEffects::EnableHDR`, `FXAA::Enable` 등의 전역 토글을 켜고 성능/화질 변화를 관찰합니다.

각 단계는 `PrototypeApp.cpp`를 확장하는 방식으로 진행하며, 범위가 커지면 별도 헤더/소스 파일로 분리하세요.
