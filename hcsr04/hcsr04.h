/**
 * @file hcsr04.h
 * @brief GPIOを用いて繋いだhc-sr04で測距を行うPythonモジュール
 * @author rm.hyphen.rs.space.slash@gmail.com
 * @date 2018-09-27
 * @sa https://www.raspberrypi.org/documentation//hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
 */

/**
 * メソッド宣言の終端
 */
#define EOM {NULL, NULL, 0, NULL}

/**
 * GPIOメモリマップ
 */
#define GPIOMEM "/dev/gpiomem"

/**
 * Level LOWの定義
 */
#define PI_LOW    0

/**
 * Level Highの定義
 */
#define PI_HIGH   1

/**
 * GPIO Pin Mode - INPUT
 */
#define PI_INPUT  0

/**
 * GPIO Pin Mode - OUTPUT
 */
#define PI_OUTPUT 1

/**
 * GPIO Pin Mode - ALT0
 */
#define PI_ALT0   4

/**
 * GPIO Pin Mode - ALT1
 */
#define PI_ALT1   5

/**
 * GPIO Pin Mode - ALT2
 */
#define PI_ALT2   6

/**
 * GPIO Pin Mode - ALT3
 */
#define PI_ALT3   7

/**
 * GPIO Pin Mode - ALT4
 */
#define PI_ALT4   3

/**
 * GPIO Pin Mode - ALT5
 */
#define PI_ALT5   2

/**
 * このモジュールで用いる、いわゆる"TRUE"
 */
#define PROC_SUCCESS 0

/**
 * このモジュールで用いる、いわゆる"FALSE"
 */
#define PROC_FAIL    -1

/**
 * チャタリング時間
 */
#define CHATTER_USEC 300 * 1000

//! 時間の構造体
typedef struct timeval TimeData;

//! パルスの状態を記憶する
typedef struct
{
    //! 現在のレベルを記録する変数
    //! @sa pulse_reader
    int EchoLevel;
    //! パルスの開始時間を記録する
    //! @sa pulse_reader
    TimeData StartTime;
    //! パルスの終了時間を記録する
    //! @sa pulse_reader
    TimeData EndTime;
} PulseStatus;

//! GPIOメモリマップ
static volatile uint32_t *gpioReg = MAP_FAILED;

/**
 * @brief GPIOポート毎にモードを設定します。
 * @param gpio GPIOポート番号
 * @param mode ポートに設定するモード
 */
static void setmode_gpio(unsigned gpio, unsigned mode);

/**
 * @brief GPIOポートの現在の値を取得します。
 * @param gpio GPIOポート番号
 * @return ポートの現在の値
 */
static unsigned read_gpio(unsigned gpio);

/**
 * @brief GPIOポートに値を設定します。
 * @param gpio GPIOポート番号
 * @param level 設定する値。LOW-HIGHの二値
 *
 * GPIOポートにLOW,HIGHいずれかの値を設定します。LOW,HIGH以外の値を
 * 設定した場合の動作は未定義です。
 */
static void write_gpio(unsigned gpio, unsigned level);

/**
 * @brief GPIOポートに対してLOWもしくはHIGHのいずれかのトリガーを作ります。
 * @param gpio GPIOポート番号
 * @param len トリガーの長さ(マイクロ秒)
 * @param level トリガーの値。LOW-HIGHの二値
 * @sa write_gpio
 *
 * GPIOポートに対して指定したマイクロ秒数のトリガーを生成します。
 * ポートの初期値は考慮しません。LOW,HIGH以外の値を設定した場合の動作は
 * 未定義です。
 */
static void trigger_gpio(unsigned gpio, unsigned len, unsigned level);

/**
 * @brief チャタリング時間まで実行待機し、GPIOポートが期待する値になっている
 * ことを確認します。
 * @param gpio GPIOポート番号
 * @param level 期待する値
 * @return 期待する値になっていればPROC_SUCCESS、そうでなければPROC_FAIL
 * @sa CHATTER_USEC
 * @sa PROC_SUCCESS
 * @sa PROC_FAIL
 * @sa read_gpio
 *
 * チャタリング時間Sleepしてから、read_gpio()を用いて現在のGPIOポート
 * の値を確認します。期待する値であればPROC_SUCCESSを、そうでなければPROC_FAIL
 * を返します。levelにLOW,HIGH以外の値を設定した場合の動作は未定義です。
 */
int wait_level_gpio(unsigned gpio, unsigned level);

/**
 * @brief GPIOポートに対してLOW->HIGH->LOWのパルスを待ち、HIGHの開始時間と終了
 * 時間を記録します。左記パルスが観測されるまで待ち続けます。
 * @param gpio GPIOポート番号
 * @param ps PulseStatusへのポインタ。パルス開始時間と修旅時間が書き込まれる
 * @param b_sig パルス待ちを中断するフラグへのポインタ
 * @return パルス観測終了すればPROC_SUCCESS、そうでなければPROC_FAIL
 * @sa PROC_SUCCESS
 * @sa PROC_FAIL
 * @sa read_gpio
 * @sa PulseStatus
 *
 * パルスが観測されるまで呼ばれたスレッド内でパルスを待ち続けます。
 * パルスの開始時間と終了時間を*psに書き込みます。短時間パルスを捉えるために
 * Sleep無しで待ち続けるため、この関数を実行している間はCPU使用率が高くなるで
 * しょう。待機を中断させたい場合は*b_sigに0以外の値を設定してください。中断
 * させた場合はこの関数はPROC_FAILを返します。
 */
int pulse_reader(unsigned gpio, PulseStatus *ps, int *b_sig);

/**
 * @brief 時間を記録したPulseStatus構造体から開始時間と終了時間の差分を取得
 * します。マイクロ秒単位で、60秒までの差分が計算できます。
 * @param ps 時間を記録した構造体へのポインタ
 * @return 時間差分（マイクロ秒）
 * @sa PulseStatus
 *
 * 時間を記録した構造体の開始時間と終了時間の差分を取得します。ただし
 * 差分は60秒までを限界としています。また開始時間，終了時間のいずれも記録して
 * いない場合の結果は未定義です。
 */
int32_t usec_delta(PulseStatus *ps);

//! Pythonの例外オブジェクト
static PyObject *hcsr04Error;

/**
 * @brief Pythonオブジェクトの定義に則って定義された関数。hc-sr04の初期化処理
 * を行う
 * @param self Pythonオブジェクト
 * @param args Python引数
 * @return Pythonオブジェクト
 */
static PyObject *init_proc(PyObject *self, PyObject *args);

//! パルス待ちを中断するフラグ
static int b_sig;

/**
 * @brief Pythonオブジェクトの定義に則って定義された関数。パルス待ちを中断する
 * @param self Pythonオブジェクト
 * @param args Python引数
 * @return Pythonオブジェクト
 * @sa measure_proc
 */
static PyObject *break_measure_proc(PyObject *self, PyObject *args);

/**
 * @brief Pythonオブジェクトの定義に則って定義された関数。距離測定を行う
 * @param self Pythonオブジェクト
 * @param args Python引数
 * @return Pythonオブジェクト
 * @sa read_gpio
 * @sa write_gpio
 * @sa trigger_gpio
 * @sa pulse_reader
 */
static PyObject *measure_proc(PyObject *self, PyObject *args);

/**
 * @brief Pythonオブジェクトの定義に則って定義された関数。hc-sr04リソースの
 * 解放を行う
 * @param self Pythonオブジェクト
 * @param args Python引数
 * @return Pythonオブジェクト
 */
static PyObject *close_proc(PyObject *self, PyObject *args);

//! Pythonに公開するメソッドを定義
static PyMethodDef methods[] = {
    {
        "init",
        init_proc,
        METH_NOARGS,
        "initialize gpio for hc-sr04."
    },
    {
        "measure",
        measure_proc,
        METH_NOARGS,
        "measure length between hc-sr04 and object."
    },
    {
        "break_measure",
        break_measure_proc,
        METH_VARARGS,
        "if given >0 value then stop measuring."
    },
    {
        "close",
        close_proc,
        METH_NOARGS,
        "close gpio for hc-sr04."
    },
    EOM
};

//! モジュールのPythonコメントを定義
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "rpi_hcsr04",
    "returns length between cup and hc-sr04.",
    -1,
    methods
};

/**
 * @brief Pythonオブジェクトの定義に則って定義された関数。Pythonモジュールとし
 * ての初期化処理を行う。
 * @return Pythonオブジェクト
 */
PyMODINIT_FUNC PyInit_rpi_hcsr04(void);
