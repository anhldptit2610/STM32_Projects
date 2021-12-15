/* 1. Overall: chương trình này sử dụng ADC1(kênh 16) và ADC2(kênh 1) để đọc giá trị cảm biến ánh sáng và
 * cảm biến nhiệt đọ(trong kit BluePill); sau đó hiển thị kết quả đo lên trên module LED 7 thanh sử dụng
 * IC TM1637. Có hai nút bấm được gán vào hai ngắt ở hai chân PB8 và PC13, mỗi một nút nhấn đại diện cho 
 * một cảm biến. Khi nhấn nút nhấn trên chân PB8, hiển thị giá trị cảm biến ánh sáng và ngược lại.
 * 
 * 2. Chú ý: Code này sử dụng hàm để config cho tần số họat động của kit là 72MHz, do vậy cấu hình Timer cũng
 * phải dựa theo tần số này(do sử dụng thanh ghi). Nếu muốn điều chỉnh, bạn hãy đọc RM(Reference Manual) của chip
 * và xem mục General Purpose Timer để biết thanh ghi nào cần cấu hình và cấu hình ra sao. 
 * 
 * 3. Q & A: 
 * 			1. Vì sao lại chọn hai cái ADC khác nhau thế? - Cho nó dễ cấu hình.
 * 			2. Vì sao lại chọn hai cái ngắt ở hai chân khác xa nhau thế? - Cho nó dễ cấu hình và đỡ bị nhầm.
 * 			3. Tại sao lại phải cấu hình kit hoạt động ở 72MHz? - Để cho nó tối ưu chức năng của kit thôi. 
 * 			   Nó hoạt động được ở 72MHz mà :)
 * 			4. Tại sao lại code theo style như này? Vì nó là quy chuẩn, bạn có thể đọc cuốn "The C Programming Language"
 * 			   để xem quy chuẩn code C ra sao.
 * 
 * 4. Giải thích một số hàm:
 * Các hàm: xxx_Config() có tác dụng cấu hình cho "xxx", chi tiết xem trong định nghĩa hàm.
 * Các hàm: xxx_IRQHandler() là các hàm xử lý khi có ngắt xảy ra, chi tiết xem trong định nghĩa hàm.
 */

/* Các thư viện cần cho chương trình. Ba thư viện rcc.h, delay.h và tm1637.h là thư viện custom, không phải thư viện chính thức
 * do hãng phát triển.
 */
#include "rcc.h"
#include "delay.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_exti.h"
#include "tm1637.h"
#include <misc.h>


/* Các define cho GPIO pin */
#define LIGHT_SENSOR_PIN					GPIO_Pin_1
#define LS_INTERRUPT_PIN					GPIO_Pin_8
#define TEMPERATURE_INTERRUPT_PIN			GPIO_Pin_13	

/* Các hằng só để tính giá trị nhiệt độ đo được từ cảm biến nhiệt độ */
#define V25 1750
#define Avg_Slope 5

GPIO_InitTypeDef GPIO_InitStructure;
ADC_InitTypeDef ADC_InitStructure;
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;


enum DISPLAY_MODE
{
	LIGHT_SENSOR,
	TEMPERATURE_SENSOR,
};

uint16_t lightsensor_value, temperature_value, AD_Value;
uint16_t mode = LIGHT_SENSOR;

void ADC_Config(void);
void GPIO_Config(void);
void EXTI_NVIC_Config(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

int main(void)
{
	RCC_Config();
	GPIO_Config();
	Timer4_Init();
	ADC_Config();
	TM1637_Init();
	EXTI_NVIC_Config();
	while (1) {	/* trong vòng lặp, ta liên tục lấy giá trị đo được của hai cảm biến, rồi check xem mode hiển thị là gì để hiển thị*/
		AD_Value = ADC_GetConversionValue(ADC1);
		temperature_value = (uint16_t)((V25 - AD_Value)/Avg_Slope + 25);	/* Công thức này ở trên mạng ehehe */
		lightsensor_value = ADC_GetConversionValue(ADC2);
		switch (mode) {
		case LIGHT_SENSOR:
			TM1637_Display(7, lightsensor_value);
			Delay_ms(2000);		/* delay cho nó đỡ nhảy số liên miên, gây khó nhìn */
			break;
		case TEMPERATURE_SENSOR:
			TM1637_Display(7, temperature_value);
			Delay_ms(2000);
			break;
		}
	}
}

/* EXTI_NVIC_Config(): cấu hình cho EXTI và NVIC, hai ngoại vi liên quan đến ngắt.
 *
 * 1. Description: hàm này cáu hình GPIO, NVIC, EXTI cho ngắt trên hai chân: PB8 và PC13. 
 *    Về EXTI, cả hai đều được cấu hình khá giống nhau khi đều là mode Interrupt, sườn xuống(Falling Edge)
 *    là sườn trigger ngắt. Hai chân này đểu được cấu hình theo kiểu IPU(Input Pull - up). Về NVIC ta chỉ cần
 *    cáu hình mức ưu tiên cho ngắt, giá trị mức ưu tiên càng lớn thì mức ưu tiên càng nhỏ và ngược lại.
 * 
 * 2. Q & A: 
 *
 */


void EXTI_NVIC_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	/* cấu hình gpio cho ngắt cảm biến ánh sáng */
	GPIO_InitStructure.GPIO_Pin = LS_INTERRUPT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* cấu hình gpio cho ngắt cảm biến nhiệt độ */
	GPIO_InitStructure.GPIO_Pin = TEMPERATURE_INTERRUPT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* cấu hình exti cho line 13(PC13 a.k.a chân ngắt của cảm biến nhiệt độ) */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
	EXTI_ClearITPendingBit(EXTI_Line13);
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* cấu hình nvic */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	/* cấu hình exti cho line 8(PB8 a.k.a chân ngắt của cảm biến ánh sáng) */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
    EXTI_ClearITPendingBit(EXTI_Line8);
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* cấu hình nvic */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line13) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line13);
		mode = TEMPERATURE_SENSOR;
	}
}

void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line8) != RESET) {
		EXTI_ClearITPendingBit(EXTI_Line8);
		mode = LIGHT_SENSOR;
	}
}

void GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	/* for ADC1, temperature sensor and light sensor */
	GPIO_InitStructure.GPIO_Pin = LIGHT_SENSOR_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* ADC_Config(): hàm thực hiện cấu hình ADC.
 * 
 * 1. Description: Cấu hình cho hai ADC: 1 và 2. ADC 1 ta dùng kênh 16(cảm biến nhiệt độ, mặc định nó là thế),
 * và ADC2 ta dùng kênh 1(PA1). Cả hai đều được cấu hình 1 kênh, không có trigger ngoài, chế độ chuyển đổi liên tục,
 * dữ liệu thu được được align theo hướng phải(chi tiết xem ở linh dưới), sau khi cáu hình, enable và hiệu chỉnh là
 * hoàn thành quá trình cáu hình ADC. Độ phân giải của ADC trên STM32 là 12 bit. Sample time là thời gian lấy mẫu,
 * càng ngắn thì giá trị thu được càng rõ. 
 * 
 * Link bài viết về data alignment: https://electronics.stackexchange.com/questions/565448/what-is-right-or-left-alignment-of-data-in-adc
 * 2. Q & A: 
 * 			1. Vì sao phải check giá trị cờ ở một số bước thế? - Các cờ này thường dùng để thông báo một sự kiện gì đó 
 * 			   hoàn thành, ví dụ cờ EOC(End of Conversion) - báo hiệu rằng quá trình chuyển đổi giá trị từ Analog -> Digital
 * 			   đã xong, do vậy ta phải check chúng để đảm bảo không bị xung đột, chẳng hạn như chưa convert xong đã đem giá trị
 * 			   đi hiển thị là không đúng.
 */

void ADC_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
#if 1
	/* ADC1 Configure */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;

	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_41Cycles5);
	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
#endif 
	/* ADC2 configure */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;

	ADC_Init(ADC2, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_41Cycles5);
	ADC_Cmd(ADC2, ENABLE);

	ADC_ResetCalibration(ADC2);
	while (ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2));

	ADC_SoftwareStartConvCmd(ADC2, ENABLE);
	while (!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));
}