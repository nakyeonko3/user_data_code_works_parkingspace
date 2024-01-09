import subprocess

class UserDataDBManagement:
    def __init__(self, CUSER_DATA_FILE_PATH) -> None:
        self.CUSER_DATA_FILE_PATH = CUSER_DATA_FILE_PATH

    def search_user_by_carNumber(self, carNumber:int) -> str or None:
        # 1)차량 번호 검색
        try:
            search_result = subprocess.run(
            [self.CUSER_DATA_FILE_PATH, "1", carNumber],
            capture_output=True,
            text=True,
            check=True 
            )
            search_output = search_result.stdout.strip()
            return search_output if search_output else None
        except subprocess.CalledProcessError as search_error:     # retruncode가 0이 아닐시, 에러 메시지 전송
            print({"message": "차량 번호 검색중 서버 오류 발생", "error": search_error.stderr})
            return None
        except Exception as e:
            print({"message": "서버 오류 발생", "error": str(e)})
            return None 
    
    def register_user_data(self, CarNumber:str, Name:str, CarType:str) -> bool:
            # 3)유저 데이터 등록 
        try:
            subprocess.run(
            [self.CUSER_DATA_FILE_PATH, "2", CarNumber, Name, CarType],
            capture_output=True,
            text=True,
            check=True
        )
            return True
        except subprocess.CalledProcessError as register_error:    # retruncode가 0이 아닐시, 에러 메시지 전송
            print({"message": "차량 번호 검색중 서버 오류 발생", "error": register_error.stderr})
            return False
        except Exception as e:
            print({"message": "유저 데이터 등록 시도중 서버 오류 발생", "error": str(e)})
            return False
            

    def register_parkingspace(self, CarNumber:str, ParkingSpace:int) -> bool:
        # 주차 번호 등록
        try:
            pregister_result = subprocess.run(
                    [self.CUSER_DATA_FILE_PATH, "3", CarNumber, str(ParkingSpace)],
                    capture_output=True,
                    text=True,
                    check=True
                )
            return True
        except subprocess.CalledProcessError as pregister_result:     
            print({"message": "주차 번호 업데이트 중 서버 오류 발생", "error": pregister_result.stderr})
            return False
        except Exception as e:
            print({"message": "서버 오류 발생", "error": str(e)})
            return False
        
    def delete_user_by_carNumber(self, CarNumber:str) -> bool:
        # 등록된 유저 삭제
        try:
            delete_result = subprocess.run(
                [self.CUSER_DATA_FILE_PATH, "4", CarNumber], 
                capture_output=True,
                text=True,
                check=True
            )
            return True
        except subprocess.CalledProcessError as delete_result:     # retruncode가 0이 아닐시, 에러 메시지 전송
            print({"message": f"차량번호 {CarNumber} 삭제중 서버 오류 발생", "error": delete_result.stderr})
            return False
        except Exception as e:
            print({"message": "데이터 삭제 중 서버 오류 발생", "error": str(e)})
            return False