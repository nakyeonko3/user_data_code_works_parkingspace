import subprocess
import json
from pprint import pprint
import pickle

RMAP_DATA = "./Rmap_data"    
MAP_DB_TEMP = "./mapdbtmp.bin"

class MAP_DB():    
    map_db = {}
    # map_db는 맵 데이터들이 저장된 dict 타입 변수임.
    # map_db 안에 'outline', 'middleLine', 'parkingSpaces', 'PathData' 가 있다.
    RMAP_DATA = RMAP_DATA
    MAP_DB_TEMP = MAP_DB_TEMP
    
    def __init__(self) -> None:
        pass
    
    @staticmethod
    def get_mapdata_from_mapdatajson(RMAP_DATA: str = RMAP_DATA):
        """
        Rmap_data 를 실행시킴.
        MapData.bin에 저장된 맵데이터를 dict 타입으로 리턴함.
        """
        try:
            map_data = subprocess.run([RMAP_DATA], capture_output=True, text=True, check=True)
            map_data = json.loads(map_data.stdout)
            return map_data      
        except subprocess.CalledProcessError as get_json_error:
            print(f"Rmap_data 실행중 오류 발생 :{get_json_error.stderr}")
        except Exception as e:
            print(f"server error get_mapdata_from_rmapbin:{str(e)}")
        

    @staticmethod
    def update_IsParkingAvailable_False_by_parkingSpacesID(id: int):
        """
        맵데이터 안의 해당 ID값을 IsParkingAvailable를 false로 만든다.
        """
        MAP_DB.map_db['parkingSpaces'][id-1]['IsParkingAvailable'] = False
        MAP_DB.store_map_db_in_mapdbtmpbin()
 
    @staticmethod
    def update_IsParkingAvailable_all_True():
        """
        맵데이터 안의 전체를 IsParkingAvailable를 true 로 만든다.
        """
        for parkingSpacesElement in MAP_DB.map_db['parkingSpaces']:
            parkingSpacesElement['IsParkingAvailable'] = True
        MAP_DB.store_map_db_in_mapdbtmpbin()
 
    @staticmethod
    def store_map_db_in_mapdbtmpbin():
        """
        mapdbtmp.bin 안에 업데이트된 맵데이터(map_db)를 저장함. 
        """
        dbfile = open(MAP_DB_TEMP, 'ab')
        pickle.dump(MAP_DB.map_db, dbfile)                    
        dbfile.close()

    @staticmethod
    def load_from_mapdbtmpbin():
        """
        mapdbtmp.bin안의 맵데이터를 map_db에 저장함.
        mapdbtmp.bin가 존재 하지 않는다면 mapdbtmp.bin파일을 새로 생성함.
        mapdbtmp.bin안에 아무런 데이터도 없다면, MapData.bin 안에 든 맵데이터를 가져와서 map_db에 저장함.
        """
        try:
            dbfile = open(MAP_DB_TEMP, 'rb')
        except FileNotFoundError:
            print(f"create new {MAP_DB_TEMP}")
            dbfile = open(MAP_DB_TEMP, 'ab')
            dbfile.close()
        except Exception as load_mapdata_error:
            print(f"load_mapdata error raised, when mapdbtmp.bin file open:{load_mapdata_error}") 
            
        try:
            MAP_DB.map_db = pickle.load(dbfile)
            print("map_db에 mapdbtmp.bin 안에 들어 있는 맵데이터를 저장함")
            dbfile.close()
        except EOFError:
            print("mapdbtmp.bin안에 저장된 맵데이터가 없습니다.")
            print(f"map_db에 MapData.bin 안에 든 맵데이터를 저장함")
            MAP_DB.map_db = MAP_DB.get_mapdata_from_mapdatajson()
        except Exception as error:
            print(f"load_mapdata error raised, when mapdbtmp.bin file read:{error}")  
    
    @staticmethod
    def get_all():
        return MAP_DB.map_db    

if __name__ == "__main__":
    MAP_DB.load_from_mapdbtmpbin()
    MAP_DB.update_IsParkingAvailable_False_by_parkingSpacesID(22)
    MAP_DB.update_IsParkingAvailable_False_by_parkingSpacesID(21)
    MAP_DB.update_IsParkingAvailable_False_by_parkingSpacesID(20)
    MAP_DB.update_IsParkingAvailable_all_True()
    pprint(MAP_DB.map_db, indent=4)

    
    
