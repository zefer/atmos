compile:
	platformio run

upload:
	platformio run -t upload

serial:
	platformio device monitor
