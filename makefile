.PHONY: all conditional sem_only clean

all: conditional sem_only

conditional:
	$(MAKE) -C conditional

sem_only:
	$(MAKE) -C sem_only

clean:
	$(MAKE) -C conditional clean
	$(MAKE) -C sem_only clean