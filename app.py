import subprocess
from flask import Flask, request, jsonify
from flask_cors import CORS
import math
import json
import re

app = Flask(__name__)
app.json.ensure_ascii = False
CORS(app)

# RMAP_DATA_FILE_PATH = "/home/tjchoi/Project/Rmap_data"
# CUSER_DATA_FILE_PATH = "/home/tjchoi/Project/Cuser_data"

RMAP_DATA_FILE_PATH = "./Rmap_data"
CUSER_DATA_FILE_PATH = "./Cuser_data"


def search_user_by_carNumber(carNumber:int):
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

    # 에러 메시지 전송
    if EventID != "1":
        return jsonify({"message": "올바르지 않은 EventID입니다."}), 400
    
    # 차량번호, 이름, 타입 중 하나라도 없을 때 웹에 에러 메시지를 전송
    required_fields = ["CarNumber", "Name", "CarType"]
    for field in required_fields:
        if field not in Data:
            return jsonify({"message": f"{field}가 입력되지 않았습니다."}), 400
    
    # 허용 문자(숫자, 한글, 영어)외의 적절치 않은 문자열(!@#$%^*) 입력시 에러 메시지 전송 .
    if not Data["CarType"].isalnum() or not Data["Name"].isalnum():
        return jsonify({"message": "이름(Name) 또는 차종(CarType)에 적절치 않은 문자열(!-#%&*)를 입력하지 마시오"}), 400
    
    # 먼저 1)차량번호를 검색함 
    # 차량 번호가 이미 등록되어 있다면, 2)검색 결과를 전송
    # 등록되어 있지 않다면, 3)유저 데이터 등록

    # 1)차량 번호 검색
    search_output = search_user_by_carNumber(Data["CarNumber"])
    # 차량 번호 검색 중 에러 발생
    if search_output == 1: 
        return jsonify({"message": "서버 오류 발생", "data": search_output})
    # 2)검색 결과를 전송
    if search_output: 
        return jsonify({"message": "검색 결과 출력", "data": search_output}), 200 
    # 3)유저 데이터 등록 
    try:
        subprocess.run(
        [CUSER_DATA_FILE_PATH, "2", Data["CarNumber"], Data["Name"], Data["CarType"]],
        capture_output=True,
        text=True,
        check=True
    )
        return jsonify({
            "message": f" 데이터 등록 완료: Name: {Data['Name']}, CarType: {Data['CarType']}, CarNumber: {Data['CarNumber']}"
        }), 200
    except subprocess.CalledProcessError as search_error:    # retruncode가 0이 아닐시, 에러 메시지 전송
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
    
    # 허용된 문자(숫자, 한글, 영어)외의 적절치 않은 문자열(!@#$%^*) 입력시 에러 메시지 전송 
    if math.isnan( Data["ParkingSpace"]):
        return jsonify({"message": "적절치 않은 문자열(!-#%&*)를 입력하지 마시오"}), 400
    
    # 주차 가능한 공간이 22대로 제한된 경우
    if int(Data["ParkingSpace"]) > 22:  
        return jsonify({"message": "주차장이 가득 찼습니다. 더 이상 주차할 수 없습니다."}), 400

    # 차량 번호 검색
    search_output = search_user_by_carNumber(Data["CarNumber"])
    
    # 차량 번호 검색 중 에러 발생
    if search_output == 1:
        return jsonify({"message": "서버 오류 발생", "data": search_output}), 500
    
    # 등록되지 않은 유저의 주차번호를 업데이트하려고 했을 경우, 에러 메시지 전송
    if not search_output:
        return jsonify({"message": f"등록되지 않은 차량번호'{Data['CarNumber']}'", "data": search_output}), 400
    
    # 주차번호 등록
    try:
        pregister_result = subprocess.run(
                [CUSER_DATA_FILE_PATH, "3", Data["CarNumber"], str(Data["ParkingSpace"])],
                capture_output=True,
                text=True,
                check=True
            )
        return jsonify( {
            "message": f"주차번호 업데이트 성공, 차량번호: {Data['CarNumber']}, 주차공간: {Data['ParkingSpace']}"
        }), 200
    except subprocess.CalledProcessError as pregister_error:     # retruncode가 0이 아닐시에도, 에러 메시지 전송
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
        return jsonify({"message": "차량번호(CarNumbe)가 입력되지 않았습니다.", "error": str(e)}), 500
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
        return jsonify({"message": "삭제중 서버 오류 발생, retruncode not 0", "error": delete_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "데이터 삭제 중 서버 오류 발생", "error": str(e)}), 500


# "/get-json-data" 엔드포인트에 대한 GET 메소드 핸들러 함수 정의
@app.route("/get-json-data", methods=["GET"])
def get_json_data():
    EventID = request.args.get("EventID")  # "EventID" 쿼리 매개변수 값을 가져옴
     
    if EventID != "3": # "EventID"가 "3"이 아닐 경우 
        return jsonify({"message":"올바르지 않은 EventID입니다."})

    try:
        map_data = subprocess.run([RMAP_DATA_FILE_PATH], capture_output=True, text=True, check=True)
        map_dict = json.loads(map_data.stdout)
        # map_dict = map_data.stdout
        return jsonify(map_dict)
    except subprocess.CalledProcessError as get_json_error:
        return jsonify({"message": "map데이터 읽는중 서버 오류 발생", "error": delete_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "서버 오류 발생", "error": str(e)}), 500
        

# 메인 실행 부분
if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)  # Flask 애플리케이션을 실행하고 외부에서 접속 가능하도록 함
