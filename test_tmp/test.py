import re

CAR_NUMBER_PATTERN="^[0-9]{2,3}[가-힣]{1}-[0-9]{4}$"
reC= re.compile(CAR_NUMBER_PATTERN)
reM = reC.match("15누가-8762")
print(reM)