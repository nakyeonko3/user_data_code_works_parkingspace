import subprocess
import json
from pprint import pprint

RMAP_DATA_FILE_PATH = "./Rmap_data"
CUSER_DATA_FILE_PATH = "./Cuser_data"
    

def get_map_data_from_bin():
    """
    process Rmap_data
    
    Return
    --------
    map_dict : dict
        get json map data from MapData.bin
    """
    try:
        map_data = subprocess.run([RMAP_DATA_FILE_PATH], capture_output=True, text=True, check=True)
        map_data = json.loads(map_data.stdout)    
    except subprocess.CalledProcessError as get_json_error:
        print(f"Rmp_data 실행중 오류 발생 {get_json_error.stderr}")
    except Exception as e:
        print(f"error {str(e)}")
    return map_data   

 


# def select_by_name(item_name : str):
#     map_data[item_name]

# def select_all():
#     return map_data
map_data = get_map_data_from_bin()
parkingSpaces = map_data['parkingSpaces']


def update_parkingSpaceID_byID(id: int, property: str = 'IsParkingAvailable'):
    global parkingSpaces
    parkingSpaces[id-1][property] = False
    
    # print(parkingSpaces[id-1]['IsParkingAvailable'])
    # pprint(parkingSpaces[id-1], indent=4)
    


if __name__ == "__main__":
    update_parkingSpaceID_byID(22)
    pprint(parkingSpaces, indent=4)

# def main():
#     rmap_data = get_json_data()
#     rmap_parkingSpaces= rmap_data['parkingSpaces']
#     # pprint(rmap_middleLines, indent=4)
#     pprint(rmap_parkingSpaces, indent=4)
    
    
