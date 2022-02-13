help: ## Show help message
	@awk 'BEGIN {FS = ":.*##"; printf "\nUsage:\n  make \033[36m\033[0m\n"} /^[$$()% 0-9a-zA-Z_-]+:.*?##/ { printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

build: ## Builds container and compiles project
	sudo docker build . -t http_sysmon 

compile: ## Compiles code using container
	sudo docker run -v ${PWD}/src:/src:z http_sysmon src/app.sh compile 

run: ## Run compiled code
	sudo docker run -v ${PWD}/src:/src:z -p 8080:8080 http_sysmon src/app.sh runapp

int: ## Interactive mode for container
	sudo docker run -it -v ${PWD}/src:/src:z -p 8080:8080 http_sysmon 
