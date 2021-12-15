/* 1. Overall: thư viện này bao gồm các hàm để sử dụng IC TM1637, cụ thể hơn
 *    là module LED7 thanh(4 cái). Chi tiết về IC này xin vui lòng đọc datasheet của 
 *    hãng(link bên dưới) để biết thêm chi tiết. Phần description bên dưới có thể
 *    sẽ không đầy đủ như datasheet của hãng.
 *    LINK: https://www.mcielectronics.cl/website_MCI/static/documents/Datasheet_TM1637.pdf
 * 
 * 2. Description: IC TM1637 có 2 chân ta cần quan tâm: DIO và CLK. Nghe qua thì ta sẽ thấy nó 
 *    khá giống I2C, nhưng thực chất giao thức này không phải I2C, do nó không sử dụng địa chỉ.
 *    Thay vào đó, ta sẽ phải sinh(generate) xung clock trên chân CLK, và truyền dữ liệu qua chân DIO.
 *    Quy trình hiển thị ký tự lên module được thực hiện qua các bước sau:
 *          1. Gửi lệnh dữ liệu(data command).
 *          2. Gửi lệnh địa chỉ(address command).
 *          3. Gửi dữ liệu(data).
 *          4. Gửi lệnh hiển thị(display command).
 *    Ví dụ: để gửi số '0' lên module, ta có thể làm như sau:
 *          1. Gửi lệnh dữ liệu: ta sẽ gửi 0x44 - viết dữ liệu lên thanh ghi hiển thị, chi tiết xem ở mục 
 *    '1. Data command setting' trong datasheet.
 *          2. Gửi lệnh địa chỉ: ta sẽ gửi 0xC0 - tương ứng với địa chỉ C0H. TM1637 có 6 địa chỉ, được đánh dấu
 *    = 0xC0 + x(x từ 0 -> 5). Chi tiết xem ở mục '2. Address command setting' trong datasheet.
 *          3. Gửi dữ liệu: ở đây do là led 7 thanh, nên ta cần gửi dữ liệu theo dạng segment, như minh họa 
 *    led 7 thanh hình dưới. Số '0' tương ứng với 0b00111111, thứ tự A->G từ phải sang trái, riêng ký tự '.'
 *    trên led thì nằm ở bên trái ngoài cùng.
 *          4. Gửi lệnh hiển thị: ở đây có hai phần. Một là cấu hình độ sáng của led, hai là cấu hình xem liệu
 *    led có được bật hay không. Bốn bit upper sẽ mặc định là 1000, bit cấu hình bật/tắt led là bit số 3, 
 *    còn ba bit ngoài cùng 2, 1, 0 để cấu hình độ sáng của led => có 8 mức sáng từ 0 tới 7. 
 *    Trong ví dụ này, ta có thể gửi 0x8F a.k.a 0b10001111- bật led và độ sáng là 7.
 * 
 *    Mỗi một lần gửi thành công data hay là lệnh, IC TM1637 sẽ trả về xung ACK bằng cách kéo chân DIO xuống mức 0
 *    trong một chu kì clock(tức là cả low và high). Tuy nhiên, để cho đơn giản thì các API trong thư viện này sẽ
 *    bỏ qua tín hiệu ACK mà không check kĩ.
 * 
 * 3. Q & A:
 */

#include "tm1637.h"


//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // B
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001,    // F
};

GPIO_InitTypeDef TM1637_GPIO;

/* TM1637_Init(): cấu hình GPIO cho TM1637. 
 */
void TM1637_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    TM1637_GPIO.GPIO_Pin = TM1637_CLK_Pin;
    TM1637_GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    TM1637_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &TM1637_GPIO);

    TM1637_GPIO.GPIO_Pin = TM1637_DIO_Pin;
    TM1637_GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    TM1637_GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &TM1637_GPIO);
}

/* TM1637_Start(): xuất tín hiệu bắt đầu truyền dữ liệu tới IC. Tại sao lại làm như này?
 * Xin mời bạn đọc mục 'Interface interpretation' và xem kĩ minh họa của hãng. 
 */
void TM1637_Start(void)
{
    TM1637_CLK_High();
    TM1637_DIO_High();
    TM1637_DIO_Low();
    TM1637_CLK_Low();
}

/* TM1637_Stop(): xuất tín hiệu dừng truyền dữ liệu tới IC.
 */
void TM1637_Stop(void)
{
    TM1637_CLK_Low();
    TM1637_DIO_Low();
    TM1637_CLK_High();
    TM1637_DIO_High();
}

void TM1637_CLK_High(void)
{
    GPIO_SetBits(GPIOA, TM1637_CLK_Pin);
}

void TM1637_CLK_Low(void)
{
    GPIO_ResetBits(GPIOA, TM1637_CLK_Pin);
}

void TM1637_DIO_High(void)
{
    GPIO_SetBits(GPIOA, TM1637_DIO_Pin);
}

void TM1637_DIO_Low(void)
{
    GPIO_ResetBits(GPIOA, TM1637_DIO_Pin);
}

/* TM1637_SendCommand(): gửi lệnh tới IC Tm1637.
 * Theo như datasheet của hãng, nếu muốn gửi lệnh ta phải sinh xung clock trên trên clk, mỗi một xung
 * ta gửi một bit lệnh. Sau khi gửi xong, ta sinh thêm 1 chu kì xung clock nữa để IC gửi ACK.
 */
void TM1637_SendCommand(unsigned char num)
{
    for (int i = 0; i < 8; i++) {
        TM1637_CLK_Low();
        GPIO_WriteBit(GPIOA, TM1637_DIO_Pin, num & 0x01);
        num = num >> 1;
        TM1637_CLK_High();
        TM1637_CLK_Low();
    }
    /* Để IC sinh tín hiệu ACK, ta làm như bên dưới */
    TM1637_CLK_High();
    TM1637_CLK_Low();
}

/* TM1637_DisplayBaseOnPos(): hiển thị ký tự dựa trên vị trí.
 * Hàm này có nội dung giống hệt như ví dụ ở mục 2 - Description. Các tham sô ta cần truyền cho hàm:
 *      1. brightness: độ sáng led. Các giá trị hợp lệ bao gồm từ 0->7.
 *      2. num: giá trị cần hiển thị. Các giá trị hợp lệ bao gồm từ 0->9, A->F, hoặc bất cứ ký tự gì bạn nghĩ ra.
 *      3. led_pos: chỉ định led nào ta muốn hiển thị. Ví dụ: led_pos = 0 tức là ta muốn hiển thị ký tự lên led ngoài
 *         cùng bên trái trên module.
 */
void TM1637_DisplayBaseOnPos(uint8_t brightness, uint8_t num, uint8_t led_pos)
{
    if (led_pos > 3)
        led_pos = 3;
    if (brightness > 7)
        brightness = 7;
    uint8_t data_TM1637, pos_TM1637, brightness_TM1637;

    data_TM1637 = digitToSegment[num];
    pos_TM1637 = led_pos | C0H;
    brightness_TM1637 = brightness | TM1637_DISPLAY_ON;

    TM1637_Start();
    TM1637_SendCommand(0x44);
    TM1637_Stop();

    TM1637_Start();
    TM1637_SendCommand(pos_TM1637);
    TM1637_SendCommand(data_TM1637);
    TM1637_Stop();
    
    TM1637_Start();
    TM1637_SendCommand(brightness_TM1637);
    TM1637_Stop();
}

/* TM1637_Display(): hiển thị số bất kì.
 * Hàm này là cải tiến của hàm ở trên, khi ta chỉ cần truyền vào hai tham số: 
 * số ta muốn hiển thị và độ sáng. Giới hạn của 'val' ở đây là các số có 4 chữ số
 * (do trên module cũng chỉ có từng đấy led :)).
 */
void TM1637_Display(uint8_t brightness, uint16_t val)
{
    uint8_t digit_arr[4] = {0}, idx = 3;

    while (val > 0) {
        digit_arr[idx--] = val % 10;
        val /= 10;
    }

    for (int i = 0; i < 4; i++)
        TM1637_DisplayBaseOnPos(brightness, digit_arr[i], i);
}