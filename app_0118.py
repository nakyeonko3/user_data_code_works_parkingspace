import subprocess
from flask import Flask, request, jsonify
from flask_cors import CORS
from map_db_0118 import MAP_DB
import re

app = Flask(__name__)
app.json.ensure_ascii = False
CORS(app)

# RMAP_DATA_FILE_PATH = "/home/tjchoi/Project/Rmap_data"
# CUSER_DATA_FILE_PATH = "/home/tjchoi/Project/Cuser_data"

RMAP_DATA_FILE_PATH = "./Rmap_data"
CUSER_DATA_FILE_PATH = "./Cuser_data"

MAP_DB.load_from_mapdbtmpbin()

def search_user_by_carNumber(carNumber:int):
    try:
        search_result = subprocess.run(
            [CUSER_DATA_FILE_PATH, "1", carNumber],
            capture_output=True,
            text=True,
            check=True 
        )
        print(search_result)
        search_output = search_result.stdout.strip()
        return search_output
    except subprocess.CalledProcessError as search_error:     
        print(f"차량 번호 검색 중 서버 오류 발생 returncode not 1, error: {search_error.stderr}")
        return None  # 서버 오류 시 None을 반환합니다.
    except Exception as e:
        print(f"서버 오류 발생 error {str(e)}")
        return None  # 서버 오류 시 None을 반환합니다.

@app.route("/UserInfo", methods=["POST"])
def handle_user():
    Data = request.json
    EventID = Data.get('EventID', '')

    # 에러 메시지 전송
    if EventID != "1":
        return jsonify({"message": "올바르지 않은 EventID입니다."}), 400
    
    # 차량번호, 이름, 타입 중 하나라도 없을 때 웹에 에러 메시지를 전송
    required_fields = ["CarNumber", "Name", "CarType"]
    for field in required_fields:
        if field not in Data:
            return jsonify({"message": f"{field}가 입력되지 않았습니다."}), 400
    
    # 이름이 2, 3, 4 글자의 한글인지 확인하고, 올바른 형식인지 검사
    if not re.match(r'^[가-힣]{2,4}$', Data["Name"]):
        name_error = {"message": "올바른 이름 형식이 아닙니다."}
    else:
        name_error = None

    # 차종이 10글자 이하의 한글인지 확인하고, 올바른 형식인지 검사
    if not re.match(r'^[가-힣]{1,10}$', Data["CarType"]):
        car_type_error = {"message": "올바른 차종 형식이 아닙니다."}
    else:
        car_type_error = None

    # 오류가 있을 경우 오류 메시지 반환
    if name_error and car_type_error:
        return jsonify({"name_error": name_error, "car_type_error": car_type_error}), 400
    elif name_error:
        return jsonify({"name_error": name_error}), 400
    elif car_type_error:
        return jsonify({"car_type_error": car_type_error}), 400

    # 차량 번호 검색
    search_output = search_user_by_carNumber(Data["CarNumber"])
    print("asd", search_output)
    # 서버 오류가 발생한 경우
    if search_output is None: 
        return jsonify({"message": "서버 오류 발생"}), 500

    # 검색 결과를 전송
    if search_output:
        parking_space_number = int(search_output)
        return jsonify({"message": "검색 결과 출력", "data": {"parkingSpace": parking_space_number}}), 200 
    print("qwezxcc", search_output)
    
    # 유저 데이터 등록 
    try:
        subprocess.run(
            [CUSER_DATA_FILE_PATH, "2", Data["CarNumber"], Data["Name"], Data["CarType"]],
            capture_output=True,
            text=True,
            check=True
        )
        return jsonify({
            "message": f"데이터 등록 완료: Name: {Data['Name']}, CarType: {Data['CarType']}, CarNumber: {Data['CarNumber']}"
        }), 200
    except subprocess.CalledProcessError as search_error:    
        return jsonify({"message": "차량 번호 검색중 서버 오류 발생", "error": search_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "유저 데이터 등록 시도중 서버 오류 발생", "error": str(e)}), 500

@app.route("/ParkingSpace", methods=["POST"])
def handle_parking_space():
    Data = request.json
    EventID = Data.get('EventID', '')

    if EventID != "2":
        return jsonify({"message": "올바르지 않은 EventID입니다."}), 400
    
    # 차량번호, 주차번호 중 하나라도 없을 때 웹에 에러 메시지를 전송
    required_fields = ["CarNumber", "ParkingSpace"]
    for field in required_fields:
        if field not in Data:
            return jsonify({"message": f"{field}가 입력되지 않았습니다."}), 400
          
    # 주차 가능한 공간이 22대로 제한된 경우
    if int(Data["ParkingSpace"]) > 22:  
        return jsonify({"message": "주차장이 가득 찼습니다. 더 이상 주차할 수 없습니다."}), 400

    # 차량 번호 검색
    search_output = search_user_by_carNumber(Data["CarNumber"])
    
    # 서버 오류가 발생한 경우
    if search_output is None:
        return jsonify({"message": "서버 오류 발생"}), 500
    
    # 등록되지 않은 유저의 주차번호를 업데이트하려고 했을 경우, 에러 메시지 전송
    if not search_output:
        return jsonify({"message": f"등록되지 않은 차량번호 '{Data['CarNumber']}'입니다.", "data": search_output}), 400
    
    # 이미 다른 사용자가 해당 주차 공간을 사용 중이면 에러 메시지 전송
    if not MAP_DB.map_db['parkingSpace'][int(Data["ParkingSpace"]) - 1]['IsParkingAvailable']:
        return jsonify({"message": f"주차공간 {Data['ParkingSpace']}은 이미 다른 사용자가 사용 중입니다."}), 400
    
    # 주차번호 등록 및 예약 상태 업데이트
    try:
        # 주차번호 업데이트 시 carNumber 인자를 전달
        MAP_DB.update_IsParkingAvailable_False_by_parkingSpaceID(int(Data["ParkingSpace"]), Data["CarNumber"])
        
        pregister_result = subprocess.run(
            [CUSER_DATA_FILE_PATH, "3", Data["CarNumber"], str(Data["ParkingSpace"])],
            capture_output=True,
            text=True,
            check=True
        )

        return jsonify( {
            "message": f"주차번호 업데이트 성공, 차량번호: {Data['CarNumber']}, 주차공간: {Data['ParkingSpace']}"
        }), 200
    except subprocess.CalledProcessError as pregister_error:
        return jsonify({"message": "주차번호 업데이트 중 서버 오류 발생", "error": pregister_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "서버 오류 발생", "error": str(e)}), 500


@app.route("/TurnOff", methods=["POST"])
def handle_turn_off():
    Data = request.json
    EventID = Data.get('EventID', '')

    if EventID != "4":
        return jsonify({"message": "올바르지 않은 EventID입니다."}), 400
    if "CarNumber" not in Data:
        return jsonify({"message": "차량번호(CarNumbe)가 입력되지 않았습니다."}), 400
    
    parkingspaceID = search_user_by_carNumber(Data["CarNumber"])
    
    if parkingspaceID is None:
        return jsonify({"message": "차량번호 검색 중 서버 오류 발생"}), 500
    
    if not parkingspaceID.isdigit():
        return jsonify({"message": "해당하는 차량번호 데이터가 없습니다."}), 400

    MAP_DB.update_IsParkingAvailable_True_by_ParkingSpaceID(int(parkingspaceID))
    
    try:
        # C 언어 프로그램을 호출하여 해당 차량번호 데이터 삭제
        delete_result = subprocess.run(
            [CUSER_DATA_FILE_PATH, "4", Data["CarNumber"]], 
            capture_output=True,
            text=True,
            check=True
        )
        return jsonify({"message": f"차량번호 {Data['CarNumber']}에 해당하는 데이터 삭제 성공"}), 200
    except subprocess.CalledProcessError as delete_error:     # retruncode가 0이 아닐시에도, 에러 메시지 전송
        return jsonify({"message": "데이터 삭제 중 서버 오류 발생", "error": delete_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "데이터 삭제 중 서버 오류 발생", "error": str(e)}), 500

# "/get-json-data" 엔드포인트에 대한 GET 메소드 핸들러 함수 정의
@app.route("/get-json-data", methods=["GET"])
def get_json_data():
    EventID = request.args.get("EventID")  # "EventID" 쿼리 매개변수 값을 가져옴

    if EventID != "3": # "EventID"가 "3"이 아닐 경우 
        return jsonify({"message":"올바르지 않은 EventID입니다."})

    return jsonify(MAP_DB.map_db)

# 메인 실행 부분
if __name__ == "__main__":
    app.run(host="0.0.0.0")  # Flask 애플리케이션을 실행하고 외부에서 접속 가능하도록 함
