#define PLAYER_1 1
#define PLAYER_2 2

#define IMU_BEETLE 3
#define GUN_BEETLE 4
#define IR_BEETLE 5

#define SLEEP_STATE 0
#define HANDSHAKE_STATE 1
#define SEND_STATE 2

#define ACK_TIMEOUT 100

//hard coded variables for each beetle
unsigned char currentPlayer = PLAYER_1;
unsigned char currentBeetle = IMU_BEETLE;

long prevTime;
bool handshakeStatus = false;

class State {
  private:
    char id;

  public:
    State(char id) {
      this->id = id;
    }

    char getID() {
      return id;
    }

    virtual void run() {}
};

State* currState;
State* nextState;

struct IMUPacket {
  byte packetID = 'I';
  int accelData[3];
  int gyroData[3];
  unsigned char compID;
  long timestamp;
  byte padding[1];
};

struct AmmoPacket {
  byte packetID = 'T';
  int ammoCount;
  unsigned char compID;
  long timestamp;
  byte padding[12];
};

struct LHealthPacket {
  byte packetID = 'R';
  int localHealth;
  unsigned char compID;
  long timestamp;
  byte padding[12];
};

struct ACKPacket {
  byte packetID = 'A';
  byte padding[19];
};

unsigned char getCompID() {
  unsigned char hival = currentPlayer << 4;
  unsigned char loval = currentBeetle;
  return hival + loval;
}

void sendIMUPacket() {
  
  IMUPacket packet;
  
  int IMUData1[3] = {10, 11, 12};
  int IMUData2[3] = {100, 150, 200};
  
  packet.accelData[0] = IMUData1[0];
  packet.accelData[1] = IMUData1[1]; 
  packet.accelData[2] = IMUData1[2]; 
  packet.gyroData[0] = IMUData2[0]; 
  packet.gyroData[1] = IMUData2[1]; 
  packet.gyroData[2] = IMUData2[2];
  packet.compID = getCompID();
  packet.timestamp = millis();

  Serial.write((byte*)&packet, sizeof(packet));
}

void sendACKPacket() {
  ACKPacket packet;
  Serial.write((byte*)&packet, sizeof(packet));
}

void sendPackets() {
  if (handshakeStatus == false) {
    sendACKPacket();
  } else if (currentBeetle == IMU_BEETLE) {
    sendIMUPacket();
  }
}  

void updateGame() {
  
}

class SleepState : public State {
  public:
    SleepState() : State(SLEEP_STATE) {}

    void run() override {}
} SleepState;

class HandshakeState : public State {
  public:
    HandshakeState() : State(HANDSHAKE_STATE) {}

    void run() override {
      long currTime = millis();
      if ((currTime - prevTime) > ACK_TIMEOUT) {
        sendPackets();
      }
    }
} HandshakeState;


class SendDataState : public State {
  public:
    SendDataState() : State(SEND_STATE) {}

    void run() override {
      sendPackets();
      nextState = &SleepState;
    }
} SendDataState;

void serialEvent() {
  int msg = Serial.read();
  if (msg == 'H') {
      nextState = &HandshakeState;
  } else if (msg == 'A') {  
      handshakeStatus = true;
      nextState = &SleepState;
  } else if (msg == 'S') {
      //updateGame(msg);
      nextState = &SendDataState; 
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  prevTime = millis();
  currState = &SleepState;
  nextState = &SleepState;  
}
  
void loop() { 
  // put your main code here, to run repeatedly:
  if (currState->getID() != nextState->getID()) {
    currState = nextState;
  }

  currState->run();
}
