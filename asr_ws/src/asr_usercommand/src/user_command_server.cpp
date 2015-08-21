#include "ros/ros.h"
#include "asr_usercommand/UserCommand.h"
#include "std_msgs/String.h"
#include "asr_usercommand/Command.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "unistd.h"

#include "/home/turtlebot2/asr_ws/devel/include/asr_usercommand/qisr.h"
#include "/home/turtlebot2/asr_ws/devel/include/asr_usercommand/msp_cmn.h"
#include "/home/turtlebot2/asr_ws/devel/include/asr_usercommand/msp_errors.h"
#include "/home/turtlebot2/asr_ws/devel/include/asr_usercommand/msp_types.h"

#include <dlfcn.h>
char rec_result[1024*4] = {0};




const char*  get_audio_file(int num)
{
	int key = num;
	while(true)//按Esc则退出
	{           
		switch(key)
		{
		case 1:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/过来.wav"; 
		case 2:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/黄山旅游.wav"; 
		case 3:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/南京高科.wav";
		case 4:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/机器人.wav";
                case 5:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/前进.wav";
                case 6:
                         return "/home/turtlebot2/asr_ws/src/asr_usercommand/wav/向左转.wav";
		default:
			continue;
		}
	}
	exit(0);
	return NULL;
}

void run_iat(const char* src_wav_filename ,  const char* param)
{
	
	const char *sessionID;
	FILE *f_pcm = NULL;
	char *pPCM = NULL;
	int lastAudio = 0 ;
	int audStat = 2 ;
	int epStatus = 0;
	int recStatus = 0 ;
	long pcmCount = 0;
	long pcmSize = 0;
	int ret = 0 ;
	sessionID = QISRSessionBegin(NULL, param, &ret);
    if (ret !=0)
	{
	    printf("QISRSessionBegin Failed,ret=%d\n",ret);
	}
	f_pcm = fopen(src_wav_filename, "rb");
	if (NULL != f_pcm) {
		fseek(f_pcm, 0, SEEK_END);
		pcmSize = ftell(f_pcm);
		fseek(f_pcm, 0, SEEK_SET);
		pPCM = (char *)malloc(pcmSize);
		fread((void *)pPCM, pcmSize, 1, f_pcm);
		fclose(f_pcm);
		f_pcm = NULL;
	}
	while (1) {
		unsigned int len = 6400;
              unsigned int audio_len = 6400;
		if (pcmSize < 12800) {
			len = pcmSize;
			lastAudio = 1;
		}
		audStat = 2;
		if (pcmCount == 0)
			audStat = 1;
		if (0) {
			if (audStat == 1)
				audStat = 5;
			else
				audStat = 4;
		}
		if (len<=0)
		{
			break;
		}
		printf("\ncsid=%s,count=%d,aus=%d,",sessionID,pcmCount/audio_len,audStat);
		ret = QISRAudioWrite(sessionID, (const void *)&pPCM[pcmCount], len, audStat, &epStatus, &recStatus);
		printf("eps=%d,rss=%d,ret=%d",epStatus,recStatus,ret);
		if (ret != 0)
			break;
		pcmCount += (long)len;
		pcmSize -= (long)len;
		if (recStatus == 0) {
			const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
			if (ret != MSP_SUCCESS)
			{
				printf("QISRGetResult Failed,ret=%d\n",ret);
				break;
			}
			if (NULL != rslt)
				{
                        
                             strcat(rec_result,rslt);
                                }
		}
		if (epStatus == MSP_EP_AFTER_SPEECH)
			break;
		usleep(150000);
	}
	ret=QISRAudioWrite(sessionID, (const void *)NULL, 0, 4, &epStatus, &recStatus);
	if (ret !=0)
	{
		printf("QISRAudioWrite Failed,ret=%d\n",ret);
	}
	free(pPCM);
	pPCM = NULL;
	while (recStatus != 5 && ret == 0) {
		const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
		if (NULL != rslt)
		{
		       strcat(rec_result,rslt);
		}
		usleep(150000);
	}
    ret=QISRSessionEnd(sessionID, NULL);
	if(ret !=MSP_SUCCESS)
	{
		printf("QISRSessionEnd Failed, ret=%d\n",ret);
	}

	printf("\n=============================================================\n");
	printf("The result is: %s\n",rec_result);
	printf("=============================================================\n");
	//usleep(100);
}



bool asr_command(asr_usercommand::UserCommand::Request  &req,
         asr_usercommand::UserCommand::Response &res)
{
  const char* login_config = "appid = 55b1aeb0, work_dir =   .  ";
  const char* param1 = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex-wb,ent=sms16k,rst=plain,rse=utf8";//直接转写，默认编码为gb2312，可以通过rse参数指定为utf8或unicode,16k音频aue=speex-wb，8k音频识别aue=speex，
  const char* param2 = "sub=iat,ssm=1,auf=audio/L16;rate=16000,aue=speex-wb,ent=sms16k,rst=json,rse=utf8";//转写为json格式，编码只能为utf8
  int ret = 0 ;
   //用户登录
  ret = MSPLogin(NULL, NULL, login_config);
  if ( ret != MSP_SUCCESS )
  {
	printf("MSPLogin failed , Error code %d.\n",ret);
  }
  ROS_INFO("Your request command number is:%ld",(long int)req.num);
  const char* asrfile=get_audio_file(req.num);

  run_iat(asrfile,param1);

  //识别结果给response.command;
  res.command=rec_result;
  ros::NodeHandle n;
  ros::Publisher command_pub = n.advertise<asr_usercommand::Command>("command", 1000);
  ros::Rate loop_rate(5);
  //把response publish出去
  int count=1;
  int jishu=5;
   while(jishu>0)
  {
    if(ros::ok())
      {
	   asr_usercommand::Command msg;
	   msg.command=res.command;
	   printf("%d   ",count);
	   printf("%s\n", msg.command.c_str());
	   command_pub.publish(msg);
	   printf("finish one time publish\n");
	   ros::spinOnce();
	   loop_rate.sleep();
	   ++count; 
      }
   --jishu;
  }
  //退出登录
  MSPLogout();
  memset(rec_result,0,sizeof(rec_result));//清空rec_result[]
  return true;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "user_command_server");
  ros::NodeHandle n;
  ros::ServiceServer service = n.advertiseService("user_command", asr_command);
  
  ROS_INFO("Ready to ASR your command; choose the audio file");
  printf("1、过来.wav\n");
  printf("2、黄山旅游.wav\n");
  printf("3、南京高科.wav\n");
  printf("4、机器人.wav\n");
  printf("5、前进.wav\n");
  printf("6、向左转.wav\n");
  ros::spin();
 
  return 0;
}