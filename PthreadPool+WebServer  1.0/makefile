.PHONY:all
all:Server CGI


Server:main.cc
	g++ -o $@ $^ -std=c++11 -lpthread
all:Server CGI

.PHONY:CGI
CGI:
	cd ./cgi;\
	make;\
	cd ../

.POHONY:clean
clean:
	rm -f Server;\
	 cd ./cgi;\
	 make clean;\
	 cd ../
	rm -rf output
.PHONY:output
output:
	make clean;make;\
	mkdir -p output/webroot/cgi;\
		cp Server output;\
		cp -rf webroot/* output/webroot;\
		cp cgi/cgi output/webroot/cgi;\
		cp start.sh output



