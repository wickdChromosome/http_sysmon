token='aiRrV41mtzxlYvKWrO72tK0LK0e1zLOZ'
for i in $(seq 1 500); do curl -d "access_token=${token}&host_name=test2" -X POST 127.0.0.1:8080/fetch; done
