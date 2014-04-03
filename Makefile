basedirs      = src 

all: build

build: 
	@for dir in $(basedirs); do (cd $$dir; make); done

clean:
	@rm -f *~
	@for dir in $(basedirs); do (cd $$dir; make clean; rm -f *~); done
