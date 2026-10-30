
#define  _LIB_MYLIB_C_
//#include "localLib.h"
#include "includes.h"


void sleep_ms(long ms) 
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR)
        ;
}

/*
 * validate_ip() : "a.b.c.d" 형식인지 검사
 *   반환값: 1 = 유효, 0 = 무효
 *   반환값: 1 = 유효, -1 = 무효
 */
int validate_ip(const char *str)
{
    int a, b, c, d;
    char tail[4] = {0};   /* 숫자 뒤에 쓰레기가 붙는지 감지 */
    char rebuilt[16];

    a=b=c=d=0;
    /* sscanf 로 점 구분 4개 숫자 + 혹시 남는 문자 파싱 */
    int n = sscanf(str, "%d.%d.%d.%d%3s", &a, &b, &c, &d, tail);

    /* 정확히 4개 필드만 있어야 하고, tail 에 아무것도 없어야 함 */
    if (n != 4 || tail[0] != '\0')
    {    
        printf("Err> in validate_ip item[%d] is short. tail[0]=%x\r\n",n, tail[0] ) ; 
        return LIOW_NOK;
    }
    /* 각 옥텟이 0~255 범위인지 확인 */
    if (a < 0 || a > 255 || b < 0 || b > 255 ||
        c < 0 || c > 255 || d < 0 || d > 255)
    {    
        printf("Err> in validate_ip value is out of range %d.%d.d.%d\r\n",a,b,c,d) ; 
        return LIOW_NOK;
    }

    /*
     * "01.02.03.04" 같이 앞에 0이 붙은 경우를 거부하고 싶다면
     * 재조합 후 원본과 비교 (선택 사항 - 필요 없으면 이 부분 제거)
     */
    snprintf(rebuilt, sizeof(rebuilt), "%d.%d.%d.%d", a, b, c, d);
    if (strcmp(rebuilt, str) != 0)
        return LIOW_NOK;

    printf("valid IP[%s]\r\n",str);
    return 0;
}
/*
 * pidof 를 이용한 중복 실행 검사
 * 반환값:  0  = 단독 실행 (정상)
 *         -1  = 이미 실행 중 (중복)
 *         -2  = 시스템 오류
 */
int check_already_running(const char *app_name) {
    char cmd[128];
    /* -x : 스크립트 이름도 매칭, -o %PPID : 자기 자신의 부모PID 제외 */
    snprintf(cmd, sizeof(cmd), "pidof -x %s -o %d", app_name, getpid());

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        perror("[ERROR] popen 실패");
        return -2;
    }

    char buf[256] = {0};
    fgets(buf, sizeof(buf), fp);
    int status = pclose(fp);

    /* pidof 는 프로세스를 찾으면 exit(0), 못찾으면 exit(1) */
    if (WEXITSTATUS(status) == 0 && strlen(buf) > 0) {
        /* 공백 기준으로 PID 파싱하여 출력 */
        printf("[ERROR] '%s' 가 이미 실행 중입니다. PID: %s", app_name, buf);
        return -1;
    }

    return 0;  /* 중복 없음 */
}

//  from iccp
char *sprt_timeval (char *out, int outSize, struct timeval *pTime)
{
	struct tm tm_info;
	char buf[64];

	localtime_r (&pTime->tv_sec, &tm_info);
	strftime (out, outSize, "%Y-%m-%d %H:%M:%S", &tm_info);
	sprintf (buf, ".%03d", (int)(pTime->tv_usec/1000));	/* include '.' in this string*/
	strncat (out, buf, outSize-1);
	return out;
}
