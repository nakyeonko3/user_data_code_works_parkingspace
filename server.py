import subprocess
from flask import Flask, request, jsonify
from flask_cors import CORS
import re
CUSER_DATA_FILE_PATH = "./Cuser_data"
# CUSER_DATA_FILE_PATH = "/home/tjchoi/Project/Cuser_data"

CAR_NUMBER_PATTERN="^[0-9]{2,3}[가-힣]{1}-[0-9]{4}$"

app = Flask(__name__)
app.json.ensure_ascii = False
CORS(app)



@app.route("/UserInfo", methods=["POST"])
def handle_user():
    Data = request.json
    EventID = Data.get('EventID', '')

    # 에러 메시지 전송
    if EventID != "1":
        return jsonify({"message": "Invalid EventID 잘못된 주소 참조."}), 400
    
    # 차량번호, 이름, 타입 중 하나라도 없을 때 웹에 에러 메시지를 전송
    required_fields = ["CarNumber", "Name", "CarType"]
    for field in required_fields:
        if field not in Data:
            return jsonify({"message": f"{field} is required!"}), 400
    
    # 허용 문자(t숫자, 한글, 영어)외의 적절치 않은 문자열(!@#$%^*) 입력시 에러 메시지 전송 
    if not Data["CarType"].isalnum() or not Data["Name"].isalnum():
        return jsonify({"message": "'이름' 또는 '차종'에 적절치 않은 문자열(!-#%&*)를 입력하지 마시오"}), 400
    
    # 허용된 패턴(차량번호 형식)을 벗어난 사용자 입력 발생시, 에러 메시지 전송 
    reC = re.compile(CAR_NUMBER_PATTERN)
    regMatch = reC.match(Data["CarNumber"])
    print(regMatch)
    if not regMatch or not Data["CarNumber"]:
        return jsonify({"message": "적절치 않은 차번호 형식입니다."}), 400
    

    # 먼저 1)차량번호를 검색함 
    # 차량 번호가 이미 등록되어 있다면, 2)검색 결과를 전송
    # 등록되어 있지 않다면, 3)유저 데이터 등록

    # 1)차량 번호 검색
    try:
        search_result = subprocess.run(
        [CUSER_DATA_FILE_PATH, "1", Data["CarNumber"]],
        capture_output=True,
        text=True,
        check=True 
        )
        search_output = search_result.stdout.strip()
        if search_output: 
            return jsonify({"message": "검색 결과 출력", "data": search_output}), 200 # 2)검색 결과를 전송
    except subprocess.CalledProcessError as search_error:     # retruncode가 0이 아닐시, 에러 메시지 전송
        return jsonify({"message": "차량 번호 검색중 서버 오류 발생", "error": search_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "서버 오류 발생", "error": str(e)}), 500


    # 3)유저 데이터 등록 
    try:
        register_result = subprocess.run(
        [CUSER_DATA_FILE_PATH, "2", Data["CarNumber"], Data["Name"], Data["CarType"]],
        capture_output=True,
        text=True,
        check=True
    )
        return jsonify({
            "message": f" 데이터 등록 완료: Name: {Data['Name']}, CarType: {Data['CarType']}, CarNumber: {Data['CarNumber']}"
        }), 200
    except subprocess.CalledProcessError as register_error:    
        return jsonify({"message": "차량 번호 검색중 서버 오류 발생", "error": register_error.stderr}), 500
    except Exception as e:
        return jsonify({"message": "유저 데이터 등록 시도중 서버 오류 발생", "error": str(e)}), 500



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
                    [CUSER_DATA_FILE_PATH, "3", Data["CarNumber"], Data["ParkingSpace"]],
                    capture_output=True,
                    text=True
                )

                if result.returncode == 0:
                    response_data = {
                        "message": f"주차장 번호 업데이트 성공, 차량번호: {Data['CarNumber']}, 주차공간: {Data['ParkingSpace']}"
                    }
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
            if "CarNumber" in Data:
                # C 언어 프로그램을 호출하여 해당 차량번호 데이터 삭제
                delete_result = subprocess.run(
                    [CUSER_DATA_FILE_PATH, "4", Data["CarNumber"]], 
                    capture_output=True,
                    text=True
                )

                if delete_result.returncode == 0:
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
        result = subprocess.run(["./Rmap_data"], capture_output=True, text=True, cwd="/home/tjchoi/Project")
        if result.returncode == 0:
            print("returncode: ", result.returncode)
            json_data = result.stdout  # 서브프로세스 실행 결과에서 JSON 데이터를 가져옴
            print("Command Output: ", result.stdout)
            return jsonify(json_data)  # JSON 데이터를 응답으로 반환
        else:
            return jsonify({"message": "데이터가 없습니다."}), 404  # 클라이언트가 요청한 리소스(파일, 데이터, 페이지)를 서버에서 찾을 수 없을 경우
    else:
        return jsonify({"message": "잘못된 EventID입니다."}), 404  # 클라이언트가 요청한 리소스(파일, 데이터, 페이지)를 서버에서 찾을 수 없을 경우

# 메인 실행 부분
if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)  # Flask 애플리케이션을 실행하고 외부에서 접속 가능하도록 함
