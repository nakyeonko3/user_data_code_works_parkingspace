import subprocess
import json
from pprint import pprint
import pickle

# RMAP_DATA = "/home/tjchoi/Project/Rmap_data"    
# MAP_DB_TEMP = "/home/tjchoi/Project/mapdbtmp.bin"

RMAP_DATA = "./Rmap_data"    
MAP_DB_TEMP = "./mapdbtmp.bin"

# MAP_DB 클래스 정의
class MAP_DB():    
    map_db = {}
    # map_db는 맵 데이터들이 저장된 dict 타입 변수임.
    # map_db 안에 'outline', 'middleLine', 'parkingSpace', 'PathData' 등의 데이터가 포함됨.
    RMAP_DATA = RMAP_DATA
    MAP_DB_TEMP = MAP_DB_TEMP

    @staticmethod
    def get_mapdata_from_mapdatajson(RMAP_DATA: str = RMAP_DATA):
        """
        Rmap_data 를 실행시킴.
        MapData.bin에 저장된 맵데이터를 dict 타입으로 리턴함.
        """
        print("map_db에 MapData.json를 저장함")
        try:
            map_data = subprocess.run([RMAP_DATA], capture_output=True, text=True, check=True)
            map_data = json.loads(map_data.stdout)
            return map_data      
        except subprocess.CalledProcessError as get_json_error:
            print(f"Rmap_data 실행중 오류 발생 :{get_json_error.stderr}")
        except Exception as e:
            print(f"server error get_mapdata_from_rmapbin:{e}")

    @staticmethod
    def update_IsParkingAvailable_False_by_parkingSpaceID(id: int, carNumber: str):
        """
        맵데이터 안의 해당 ID값을 IsParkingAvailable를 false로 만들고,
        이전에 예약했던 주차 공간이 있다면 해당 공간을 true로 설정한다.
        """
        # 이전에 예약된 공간을 찾아 True로 설정
        for space in MAP_DB.map_db['parkingSpace']:
            # 각 주차 공간에서 'ReservedBy' 키가 있는지 확인하고, 해당 차량 번호에 예약된 경우
            if 'ReservedBy' in space and space['ReservedBy'] == carNumber:
                # 해당 주차 공간을 사용 가능하게 설정
                space['IsParkingAvailable'] = True
                # 예약 정보 초기화
                space['ReservedBy'] = None
                break
              
        # 새로운 주차 공간을 False로 설정
        new_space = MAP_DB.map_db['parkingSpace'][id-1]
        # 새로운 주차 공간이 사용 가능한 경우
        if new_space['IsParkingAvailable']:
            # 주차 공간을 사용 불가능하게 설정
            new_space['IsParkingAvailable'] = False
            # 주차 공간에 차량 번호 예약 설정
            new_space['ReservedBy'] = carNumber
            # 업데이트된 맵 데이터를 파일에 저장
            MAP_DB.store_map_db_in_mapdbtmpbin()
        else:
            # 주차 공간이 이미 예약된 경우 에러 메시지 발생
            raise ValueError(f"Parking space {id} is already reserved")

    @staticmethod
    def update_IsParkingAvailable_True_by_ParkingSpaceID(id: int):
        """
        맵데이터 안의 해당 ID값을 IsParkingAvailable를 true 로 만든다.
        """
        # 주어진 ID에 해당하는 주차 공간의 예약 가능 여부를 True로 설정
        MAP_DB.map_db['parkingSpace'][id-1]['IsParkingAvailable'] = True
        # 해당 주차 공간의 예약 정보를 초기화
        MAP_DB.map_db['parkingSpace'][id-1]['ReservedBy'] = None
        # 변경된 맵 데이터를 임시 파일에 저장
        MAP_DB.store_map_db_in_mapdbtmpbin()

    @staticmethod
    def store_map_db_in_mapdbtmpbin():
        """
        mapdbtmp.bin 안에 업데이트된 맵데이터(map_db)를 저장함. 
        """
        # mapdbtmp.bin 파일에 맵 데이터 저장 과정 시작을 알림
        print("MAP_DB store in mapdbtmp.bin")
        # mapdbtmp.bin 파일을 추가 모드('ab')로 열기
        with open(MAP_DB_TEMP, 'ab') as dbfile:
            # map_db 데이터를 파일에 피클링하여 저장
            pickle.dump(MAP_DB.map_db, dbfile)        

    @staticmethod
    def load_from_mapdbtmpbin():
        """
        mapdbtmp.bin안의 맵데이터를 map_db에 저장함.
        mapdbtmp.bin가 존재 하지 않는다면 mapdbtmp.bin파일을 새로 생성함.
        mapdbtmp.bin안에 아무런 데이터도 없다면, MapData.bin 안에 든 맵데이터를 가져와서 map_db에 저장함.
        """
        try:
            # mapdbtmp.bin 파일을 이진 읽기 모드로 열기
            with open(MAP_DB_TEMP, 'rb') as dbfile:
                # 파일에서 맵 데이터를 불러와 map_db에 저장
                MAP_DB.map_db = pickle.load(dbfile)
        except FileNotFoundError:
            # 파일이 존재하지 않을 경우의 처리
            print(f"{MAP_DB_TEMP} FileNotFound")
            # 새 파일 생성에 관한 메시지 출력
            print(f"create new {MAP_DB_TEMP}")
            # MapData.json에서 맵 데이터를 가져와 map_db에 저장
            MAP_DB.map_db = MAP_DB.get_mapdata_from_mapdatajson()
            # map_db의 내용을 mapdbtmp.bin 파일에 저장
            MAP_DB.store_map_db_in_mapdbtmpbin()
        except Exception as load_mapdata_error:
            # 파일 로딩 중 다른 예외가 발생한 경우의 처리
            print(f"load_mapdata error raised, when mapdbtmp.bin file open:{load_mapdata_error}") 

    @staticmethod
    def get_all():
        """
        MAP_DB 클래스의 map_db 딕셔너리를 반환하는 메서드.
        """
        return MAP_DB.map_db    

