# ReplayNet


## ReplayBot

* bwapi.ini 설정 방법은  트렐로, BasicBot wiki, bwapi-data 폴더 내 bwapi.ini 파일 참고

* C:\StarCraft\bwapi-data\write\mapInfo 폴더를 생성해 두어야 에러가 안남. 수정 예정

* ReplayBotDLL 과 ReplayBotEXE 은 대부분 소스코드 중복. 수정 예정


## ReplayCopy

* 사용법 : ReplayCopy.exe DestinationFolder 

* 모든 하위폴더를 순회하며 *.rep 파일을 DestinationFolder 로 복사해넣으면서 파일이름을 간단한 숫자로 변경한다

* ReplayCopyResult.csv 파일에 다음 내용을 저장한다

    * 원래 파일경로, 원래 파일이름, 원래 파일생성날짜, 변경된 파일이름, 원래 파일크기


