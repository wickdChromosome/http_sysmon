token='3qb2XNa8CSzDVUSVTmSi1jkJFfTlk7bl'
for i in $(seq 1 500); do curl -d "access_token=${token}&host_name=test1" -X POST 127.0.0.1:8080/fetch; done
