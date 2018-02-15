SHELL:=/bin/bash
CC="gcc"
APP="battery_notify"
PIDFILE="/run/user/$$UID/$(APP).pid"
compile:
	$(CC) ${APP}.c main.c -o $(APP)

debug:
	$(CC) -D BEBUG ${APP}.c main.c -o $(APP) -g
	test -f $(PIDFILE) && (cat $(PIDFILE) |\
		xargs -IPID kill PID) || \
    	rm $(PIDFILE)
install:
	mv ${APP} bin/

clean:
	test -f $(PIDFILE) && (cat $(PIDFILE) | xargs -IPID kill PID) || rm $(PIDFILE)
	rm *.log &&
		rm bin/${APP}

