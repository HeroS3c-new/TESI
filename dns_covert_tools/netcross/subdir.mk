################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
$(ROOT)/Buffer.cpp \
$(ROOT)/Client.cpp \
$(ROOT)/ConfParser.cpp \
$(ROOT)/DNSPacket.cpp \
$(ROOT)/Endpoint.cpp \
$(ROOT)/Exception.cpp \
$(ROOT)/Link.cpp \
$(ROOT)/LosslessLink.cpp \
$(ROOT)/NSLink.cpp \
$(ROOT)/ProxyEndpoint.cpp \
$(ROOT)/RedirectorEndpoint.cpp \
$(ROOT)/SessionServer.cpp \
$(ROOT)/Socket.cpp \
$(ROOT)/Timer.cpp \
$(ROOT)/TunTapEndpoint.cpp \
$(ROOT)/main.cpp 

OBJS += \
./Buffer.o \
./Client.o \
./ConfParser.o \
./DNSPacket.o \
./Endpoint.o \
./Exception.o \
./Link.o \
./LosslessLink.o \
./NSLink.o \
./ProxyEndpoint.o \
./RedirectorEndpoint.o \
./SessionServer.o \
./Socket.o \
./Timer.o \
./TunTapEndpoint.o \
./main.o 

DEPS += \
${addprefix ./, \
Buffer.d \
Client.d \
ConfParser.d \
DNSPacket.d \
Endpoint.d \
Exception.d \
Link.d \
LosslessLink.d \
NSLink.d \
ProxyEndpoint.d \
RedirectorEndpoint.d \
SessionServer.d \
Socket.d \
Timer.d \
TunTapEndpoint.d \
main.d \
}


# Each subdirectory must supply rules for building sources it contributes
%.o: $(ROOT)/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	@echo g++ -O3 -Wall -c -fmessage-length=0 -o$@ $<
	@g++ -O3 -Wall -c -fmessage-length=0 -o$@ $< && \
	echo -n $(@:%.o=%.d) $(dir $@) > $(@:%.o=%.d) && \
	g++ -MM -MG -P -w -O3 -Wall -c -fmessage-length=0  $< >> $(@:%.o=%.d)
	@echo 'Finished building: $<'
	@echo ' '


