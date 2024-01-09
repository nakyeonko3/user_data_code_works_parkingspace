import re

CAR_NUMBER_PATTERN = r"^[0-9]{2,3}[가-힣]{1}-[0-9]{4}$"

def validate_car_number(car_number):
    return bool(re.match(CAR_NUMBER_PATTERN, car_number))

def validate_inputs(data):
    required_fields = ["CarNumber", "Name", "CarType"]
    for field in required_fields:
        if field not in data:
            return False
    if not data["Name"].isalnum() or not data["CarType"].isalnum():
        return False
    if not validate_car_number(data["CarNumber"]):
        return False
    return True