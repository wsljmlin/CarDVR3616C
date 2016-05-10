# Hisilicon Hi3516  Makefile

include Makefile.param

STRIP=arm-hisiv100nptl-linux-strip

# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)

.PHONY : all clean cleanstream

all: $(TARGET)

$(TARGET):$(OBJ) $(BASE_OBJ) $(CTRL_OBJ) $(COMM_OBJ)
	@echo "------------linking--------------"
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS) -Wl,-Map,ipc_main.map
	$(STRIP) $@
	@rm -f $(OBJ)
	@echo "***********************************************"
	@echo "SENSOR_TYPE	:$(SENSOR_TYPE)"	
	@echo "TARGET		:$(TARGET)"	
	@echo "***********************************************"

$(OBJ):%.o:%.c
	@echo "--------compileing $@-----------"
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(BASE)/*.o
	@rm -f $(CTRL)/*.o
	@rm -f $(COMM)/*.o
	@rm -f *.map

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
	@rm -f *.adpcm
