token='aiRrV41mtzxlYvKWrO72tK0LK0e1zLOZ'
for i in $(seq 1 100); do curl -d "access_token=${token}&load_avg=0.2&ram_perc=0.5&host_name=test3&unix_time=${i}" -X POST 127.0.0.1:8080/report; done

