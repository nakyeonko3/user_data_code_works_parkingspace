import subprocess
from flask import Flask, request, jsonify
from flask_cors import CORS
import math
import json
import re

app = Flask(__name__)
app.json.ensure_ascii = False
CORS(app)

# tmp_dict = {}

RMAP_DATA_FILE_PATH = "./Rmap_data"
CUSER_DATA_FILE_PATH = "./Cuser_data"

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
