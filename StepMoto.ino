//电机运行的拍数
int volatile p = 0;
int volatile InterruptCount = 0;
//电机运行速度
//正值电机顺时针，负值电机逆时针；
int Speed = 0;

int SpeedValue = 1000;

//电机运行的步数；
long volatile StepCount = 0;

//电机的三个引脚所对应的Arduino上管脚
const int A=10, B=9, C=8;

//用于保存输入数据的字符串
String inputString = "";

//字符串是否已经接收完全
boolean stringComplete = false;
boolean RunOk = true;

void setup()
{
  pinMode(8, OUTPUT);//初始化8,9,10为电机的三相
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT_PULLUP);//UNO中2,3为中断，为了用低电平触发中断采用内部上拉电阻拉高至5V
  pinMode(3, INPUT_PULLUP);
  Serial.begin(9600);//打开串口并设置串口波特率为9600
  Serial.println("begin");
}

void loop()
{
  if (stringComplete == true)//接收完毕
  {
    Serial.println("loop");
    stringComplete = false;
    ExecuteOrder();//执行接收内容
    inputString = "";//清空保存字符
  }
}

//执行运行命令
void ExecuteOrder()
{
  Serial.print("inputString=");
  Serial.println(inputString);

  char a = inputString.charAt(0);
  Serial.print("charAt(0) = ");
  Serial.println(a);
  switch (a)
  {
  case '+':
    StepCount = inputString.toInt();
    Serial.print("StepCount=");
    Serial.println(StepCount);
    Speed = SpeedValue;
    RunStep();
    break;
  case '-':
    StepCount = abs(inputString.toInt());
    Serial.print("StepCount=");
    Serial.println(StepCount);
    Speed = SpeedValue * (-1);
    RunStep();
    break;
  case 'H':
    StepCount = 99999;
    Serial.print("GoHome");
    Serial.println(StepCount);
    Speed = SpeedValue * (-1);
    RunStep();
    break;
  }
}

//运行步数
void RunStep()//(int Direction)
{
  //Speed *= Direction;
  while (StepCount>0)// && RunOk==true)
  {
    one_step(Speed); //Speed控制方向和速度
    StepCount--;
    Serial.print("RunStep:StepCount:");
    Serial.println(StepCount);

    //串口数据监视紧急停止
	   while (Serial.available()>0)
	  {
	    char stopChar = Serial.read();//将串口读取字符依次保存在inChar
	    inputString += (char)stopChar;
	    delay(10);
	    if (stopChar == 13)//inChar最后以换行符结束的话接收字符串设置为接收完毕
	    {
	      StepCount=0;
	    }
	  }
  }
}

//串口读取事件
void serialEvent()
{
  while (Serial.available()>0)
  {
    char inChar = Serial.read();//将串口读取字符依次保存在inChar
    inputString += (char)inChar;
    delay(10);
    if (inChar == 13)//inChar最后以换行符结束的话接收字符串设置为接收完毕
    {
      Serial.println(inputString);
      stringComplete = true;
    }
  }
}

//让电机走一步
//参数t是电机转动方向；
void one_step(int t)
{
  switch (p)
  {
  case 0:
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, LOW);
    break;
  case 1:
    digitalWrite(A, HIGH);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
    break;
  case 2:
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, LOW);
    break;
  case 3:
    digitalWrite(A, LOW);
    digitalWrite(B, HIGH);
    digitalWrite(C, HIGH);
    break;
  case 4:
    digitalWrite(A, LOW);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
    break;
  case 5:
    digitalWrite(A, HIGH);
    digitalWrite(B, LOW);
    digitalWrite(C, HIGH);
    break;
  }
  if (t < 0)
  {
    p++;
    if (p > 5) { p = 0; }
  }
  else
  {
    p--;
    if (p < 0) { p = 5; }
  }
  attachInterrupt(0, Stop_Left, LOW);//低电平触发中断
  attachInterrupt(1, Stop_Right, LOW);
  delay(abs(t));
}

//左侧限位开关中断程序
void Stop_Left()
{
  
  InterruptCount += 1;//中断次数加1

  if (InterruptCount < 3)
  {
    Serial.println("Stop:Left");
    RunOk = false;
    Serial.println(RunOk);
  }
  else
  {
    RunOk = true;
    StepCount = 1;
    Speed = SpeedValue;
    RunStep();
  }
}

//右侧限位开关中断程序
void Stop_Right()
{
  InterruptCount += 1;//中断次数加1

  if (InterruptCount < 3)
  {
    Serial.println("Stop:Right");
      RunOk = false;
      Serial.println(RunOk);
  }
  else
  {
      RunOk = true;
      StepCount = 1;
      Speed = SpeedValue * (-1);
      RunStep();
  }
}
