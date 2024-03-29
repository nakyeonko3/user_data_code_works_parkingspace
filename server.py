import subprocess
from flask import Flask, request, jsonify
from flask_cors import CORS
import json
from map_db import MAP_DB

# CUSER_DATA_FILE_PATH = "/home/tjchoi/Project/Cuser_data"
CUSER_DATA_FILE_PATH = "./Cuser_data"

app = Flask(__name__)
app.json.ensure_ascii = False
CORS(app)

# 추가됨
MAP_DB.load_from_mapdbtmpbin()

# 추가됨
def get_parkingspaceID_by_carNumber(carNumber):
    # 1)차량 번호 검색
    try:
        search_result = subprocess.run(
        [CUSER_DATA_FILE_PATH, "1", carNumber],
        capture_output=True,
        text=True,
        check=True 
        )
        search_output = search_result.stdout.strip()
        return search_output
    except subprocess.CalledProcessError as search_error:     # retruncode가 0이 아닐시, 에러 메시지 전송
        print(f"차량 번호 검색중 서버 오류 발생 returncode not 1, error: {search_error.stderr}")
        return 1
    except Exception as e:
        print(f"서버 오류 발생 error {str(e)}")
        return 1



@app.route("/UserInfo", methods=["POST"])
def handle_user():
    Data = request.json
    EventID = Data.get('EventID', '')

    # 이벤트 ID에 따라 분기 처리
    if EventID == "1":
        try:
            # 차량 번호가 제공되었는지 확인
            if "CarNumber" in Data:
                # 차량 번호로 검색 시도
                search_result = subprocess.run(
                    [CUSER_DATA_FILE_PATH, "1", Data["CarNumber"]],
                    capture_output=True,
                    text=True
                )
                # 검색 결과를 처리합니다.
                if search_result.returncode == 0 and search_result.stdout.strip():
                    # 차량 번호에 해당하는 데이터가 있을 경우 클라이언트로 데이터 전송
                    search_output = search_result.stdout.strip()
                    return jsonify({"message": "검색 결과", "data": search_output}), 200
                else:
                    # 차량 번호에 해당하는 데이터가 없거나 오류 발생 시
                    if "Name" in Data and "CarType" in Data:
                        # 데이터 등록 시도
                        register_result = subprocess.run(
                            [CUSER_DATA_FILE_PATH, "2", Data["CarNumber"], Data["Name"], Data["CarType"]],
                            capture_output=True,
                            text=True
                        )
                        if register_result.returncode == 0:
                            return jsonify({"message": f"유저 데이터 등록 완료, 이름: {Data['Name']}, 차종: {Data['CarType']}, 차량번호: {Data['CarNumber']}"}), 200
                        else:
                            return jsonify({"message": "유저 데이터 등록 실패", "error": register_result.stderr}), 500
                    else:
                        return jsonify({"message": "추가 정보가 필요합니다. 이름과 차종을 제공해야 합니다."}), 400
            else:
                return jsonify({"message": "차량번호를 제공해야 합니다."}), 400
        except Exception as e:
            return jsonify({"message": "서버 오류 발생", "error": str(e)}), 500

@app.route("/ParkingSpace", methods=["POST"])
def handle_parking_space():
    Data = request.json
    EventID = Data.get('EventID', '')

    # 이벤트 ID에 따라 분기 처리
    if EventID == "2":
        try:
            # 데이터에 CarNumber와 ParkingSpace가 포함되어 있는지 확인
            if "CarNumber" in Data and "ParkingSpace" in Data:
                # 주차 가능한 공간이 22대로 제한되어 있다면, 주차 가능한 공간을 초과하는 경우에만 오류 메시지 반환
                if int(Data["ParkingSpace"]) > 22:  # 주차 가능한 공간이 22대로 제한된 경우
                    return jsonify({"message": "주차장이 가득 찼습니다. 더 이상 주차할 수 없습니다."}), 400

                # 주차 가능한 공간이 있을 경우 주차장 번호 업데이트 수행
                result = subprocess.run(
                    [CUSER_DATA_FILE_PATH, "3", Data["CarNumber"], str(Data["ParkingSpace"])],
                    capture_output=True,
                    text=True
                )

                if result.returncode == 0:
                    response_data = {
                        "message": f"주차장 번호 업데이트 성공, 차량번호: {Data['CarNumber']}, 주차공간: {Data['ParkingSpace']}"
                    }
                    
                    #추가됨
                    MAP_DB.update_IsParkingAvailable_False_by_parkingSpaceID(int(Data["ParkingSpace"]))
                    return jsonify(response_data), 200
                else:
                    return jsonify({"message": "주차장 번호 업데이트 실패"}), 500
            else:
                return jsonify({"message": "차량번호와 주차공간을 제공해야 합니다."}), 400
        except Exception as e:
            return jsonify({"message": "주차장 번호 업데이트 실패", "error": str(e)}), 500

    return jsonify({"message": "올바르지 않은 EventID입니다."}), 400

@app.route("/TurnOff", methods=["POST"])
def handle_turn_off():
    Data = request.json
    EventID = Data.get('EventID', '')

    # 이벤트 ID에 따라 분기 처리
    if EventID == "4":
        try:
            # 추가됨
            parkingspaceID = get_parkingspaceID_by_carNumber(Data["CarNumber"])
            print(parkingspaceID)
            MAP_DB.update_IsParkingAvailable_True_by_ParkingSpaceID(int(parkingspaceID))
            if "CarNumber" in Data:
                # C 언어 프로그램을 호출하여 해당 차량번호 데이터 삭제
                delete_result = subprocess.run(
                    [CUSER_DATA_FILE_PATH, "4", Data["CarNumber"]], 
                    capture_output=True,
                    text=True
                )

                if delete_result.returncode == 0:
                    #추가됨
                    return jsonify({"message": f"차량번호 {Data['CarNumber']}에 해당하는 데이터 삭제 성공"}), 200
                else:
                    return jsonify({"message": "데이터 삭제 실패", "error": delete_result.stderr}), 500
        except Exception as e:
            return jsonify({"message": "데이터 삭제 중 서버 오류 발생", "error": str(e)}), 500

    return jsonify({"message": "올바르지 않은 EventID입니다."}), 400

# "/get-json-data" 엔드포인트에 대한 GET 메소드 핸들러 함수 정의
@app.route("/get-json-data", methods=["GET"])
def get_json_data():

    EventID = request.args.get("EventID")  # "EventID" 쿼리 매개변수 값을 가져옴
    # 클라이언트로부터 받은 EventID 출력
    print(f"Received EventID from client: {EventID}")
    
    if EventID == "3":  # "EventID"가 "3"일 경우
        # result = subprocess.run(["./Rmap_data"], capture_output=True, text=True, cwd="/home/tjchoi/Project")
        return jsonify(MAP_DB.map_db)  # JSON 데이터를 응답으로 반환


# 메인 실행 부분
if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)  # Flask 애플리케이션을 실행하고 외부에서 접속 가능하도록 함
