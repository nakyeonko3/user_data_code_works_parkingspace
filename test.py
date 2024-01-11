from flask import Flask, request, jsonify
import json
import subprocess

RMAP_DATA_FILE_PATH = "./Rmap_data"

app = Flask(__name__)

@app.route("/")
def index():
    return "hello, world!"

@app.route("/get-json-data", methods=["GET"])
def get_json_data():

    EventID = request.args.get("EventID")  # "EventID" 쿼리 매개변수 값을 가져옴
    
    # 클라이언트로부터 받은 EventID 출력
    print(f"Received EventID from client: {EventID}")
    
    if EventID == "3":  # "EventID"가 "3"일 경우
        result = subprocess.run([RMAP_DATA_FILE_PATH], capture_output=True, text=True)
        if result.returncode == 0:
            print("returncode: ", result.returncode)
            json_data = result.stdout  # 서브프로세스 실행 결과에서 JSON 데이터를 가져옴
            print("Command Output: ", result.stdout)
            return jsonify(json_data)  # JSON 데이터를 응답으로 반환
        else:
            return jsonify({"message": "데이터가 없습니다."}), 404  # 클라이언트가 요청한 리소스(파일, 데이터, 페이지)를 서버에서 찾을 수 없을 경우
    else:
        return jsonify({"message": "잘못된 EventID입니다."}), 404  # 클라이언트가 요청한 리소스(파일, 데이터, 페이지)를 서버에서 찾을 수 없을 경우

if __name__ == "__main__":
    app.run(port=50501, debug=True)