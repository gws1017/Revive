# Revive
2022 한국공학대학교 게임공학과 팀 Revive의 졸업작품인 Revive 입니다.

원본 팀프로젝트 저장소에서 개인적으로 추가적인 수정 작업을 하기위해 Fork한 저장소입니다.

***

### 팀원
[최준혁](https://github.com/Mari-Jun) - 클라이언트
* 클라이언트 프레임워크 구현 (Actor Component 형태의 프레임워크, Asset Manager)
* 렌더링 프레임워크 구현 (DirectX12를 이용한 렌더링 엔진, Octree, LOD, Post Processing)
* 물리(충돌) 구현
* 서버와 통신 클라이언트 구조 설계 및 구현
* Revive 게임 Visual

[양원석](https://github.com/undugy) - 서버
* 게임 서버 구현
* 클라이언트와 서버 간의 송수신 구현
* DB와 연동 구현

[박해성](https://github.com/gws1017) - 클라이언트
* **FBX파일 익스포팅**과 Load작업을 통한 **애니메이션 데이터 저장**
* FMOD를 사용한 사운드 처리
* 저장된 애니메이션 데이터를 **쉐이더로 전달** 후 **정점 처리**를 통해 애니메이션 구현
* 플레이어, 몬스터의 체력 처리, 게임 종료 및 승리 등 전반적인 Game Logic 구현
* 플레이어 캐릭터 **State Machine Class를 통한 자연스러운 애니메이션 전환** 구현
* **서버**에서 전달받은 **패킷 클라이언트에 적용**
***

### 개발 환경
* DirectX 12,  C++
* IOCP 서버

***
### 클라이언트 개발 일지 (박해성)
* 주차별 개발 내용을 보고 싶으시면 [여기](https://github.com/gws1017/Revive/wiki) 를 눌러주세요

### Youtube
[![Youtube Link](http://img.youtube.com/vi/nAgPvVkmwj8/0.jpg)](https://www.youtube.com/watch?v=nAgPvVkmwj8)

### 인게임 이미지
![image](https://user-images.githubusercontent.com/34498116/183625957-f3d069c6-801e-4d65-beab-49cb1667e24e.png)
![image](https://user-images.githubusercontent.com/34498116/183626309-07417aec-d9a9-4cce-9a78-0d89e0887e7b.png)
![image](https://user-images.githubusercontent.com/34498116/183626383-3d23870b-b411-4941-81e1-10ad6291309b.png)
![image](https://user-images.githubusercontent.com/34498116/183626435-1798358a-f61c-49a9-85da-5d36bbf12d1a.png)




