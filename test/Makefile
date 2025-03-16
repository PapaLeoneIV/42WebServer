all: docker-compose.yml Dockerfile nginx.conf
	clear
	docker compose up --build

up:
	docker compose up

fclean: 
	docker stop $$(docker ps -aq)
	docker rm $$(docker ps -aq)

list:
	docker ps
