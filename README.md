# ReplayNet

* 개요 : Starcraft : Broodwar 1.16.1 버전의 리플레이 파일을 전처리하고 변환하기 위한 프로그램들

## ReplayBotDLL / ReplayBotEXE

* 개요 : 리플레이 파일을 텍스트 파일로 변환하는 프로그램

* bwapi.ini 설정 방법은  트렐로, BasicBot wiki, bwapi-data 폴더 내 bwapi.ini 파일 참고

* C:\StarCraft\bwapi-data\write\mapInfo 폴더를 생성해 두어야 에러가 안남. 수정 예정

* ReplayBotDLL 과 ReplayBotEXE 은 대부분 소스코드 중복. 수정 예정

## ReplayCopyEXE

* 개요 : 리플레이 파일들을 일괄 복사하며 목록을 만드는 프로그램

* 사용법 : ReplayCopyEXE.exe "SourceFolderPath" "DestinationFolderPath"

* SourceFolderPath 의 모든 하위폴더를 순회하며 *.rep 파일을 DestinationFolderPath 로 복사해넣는다

    * 파일이름을 간단한 숫자 (현재 time in milliseconds) 로 변경한다

* DestinationFolderPath 에 list.csv 파일을 생성하여 다음 내용을 저장한다

    * SourceFolderPath, 원본 폴더 경로, 원본 파일명, 원본 파일 생성일시, DestinationFolderPath 명, 신규 파일 이름, 파일 크기, 파일 MD5 해시값, 복사결과



