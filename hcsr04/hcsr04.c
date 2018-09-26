#include <Python.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "hcsr04.h"

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define GPPUD     37
#define GPPUDCLK0 38
#define GPPUDCLK1 39

#define PI_BANK (gpio>>5)
#define PI_BIT  (1<<(gpio&0x1F))

#define PULSE_USEC 10    // microsecond

#define GPIO_TRIG 17    // pin number 11
#define GPIO_ECHO 27    // pin number 13

#define DELTA_MAXSEC 60
#define SONIC_MM_PER_USEC 0.34

#define SEC_TO_USEC(x) (x) * 1000 * 1000

static void setmode_gpio(unsigned gpio, unsigned mode)
{
    int reg, shift;

    reg   =  gpio/10;
    shift = (gpio%10) * 3;

    gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

static unsigned read_gpio(unsigned gpio)
{
    return (
        ((*(gpioReg + GPLEV0 + PI_BANK) & PI_BIT) != 0) ?
            PI_HIGH : PI_LOW
    );
}

static void write_gpio(unsigned gpio, unsigned level)
{
    switch(level)
    {
        case PI_LOW:
            {
                *(gpioReg + GPCLR0 + PI_BANK) = PI_BIT;
                break;
            }
        case PI_HIGH:
            {
                *(gpioReg + GPSET0 + PI_BANK) = PI_BIT;
                break;
            }
    }
}

static void trigger_gpio(unsigned gpio, unsigned len, unsigned level)
{
    write_gpio(gpio, (level == PI_LOW)? PI_LOW : PI_HIGH);
    usleep(len);
    write_gpio(gpio, (level == PI_LOW)? PI_HIGH : PI_LOW);
}

int wait_level_gpio(unsigned gpio, unsigned level)
{
    usleep(CHATTER_USEC);
    return
        (read_gpio(gpio) == level) ?
            PROC_SUCCESS : PROC_FAIL;
}

int pulse_reader(unsigned gpio, PulseStatus *ps, int *b_sig)
{
    while(read_gpio(gpio) == PI_LOW)
    {
        if(*b_sig != 0)
        {
            return PROC_FAIL;
        }
    }
    
    gettimeofday(&(ps->StartTime), NULL);
    while(read_gpio(gpio) == PI_HIGH)
    {
        if(*b_sig != 0)
        {
            return PROC_FAIL;
        }
    }
    gettimeofday(&(ps->EndTime), NULL);

    return PROC_SUCCESS;
}

int32_t usec_delta(PulseStatus *ps)
{
    int32_t t1sec = (*ps).StartTime.tv_sec % DELTA_MAXSEC;
    int32_t t2sec = (*ps).EndTime.tv_sec % DELTA_MAXSEC;
    return ((SEC_TO_USEC(t2sec) + (*ps).EndTime.tv_usec)
            - (SEC_TO_USEC(t1sec) + (*ps).StartTime.tv_usec));
}

static PyObject *init_proc(PyObject *self, PyObject *args)
{
    int fd;

    if(gpioReg != MAP_FAILED)
    {
        PyErr_SetString(hcsr04Error, "already initialized.");
        return NULL;
    }

    fd = open(GPIOMEM, O_RDWR | O_SYNC);
    if(fd < 0)
    {
        PyErr_Format(hcsr04Error, "cannot open " GPIOMEM ". %d : %s", errno, strerror(errno));
        return NULL;
    }
    gpioReg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if(gpioReg == MAP_FAILED)
    {
        PyErr_Format(hcsr04Error, "cannot mmap " GPIOMEM ". %d : %s", errno, strerror(errno));
        return NULL;
    }

    setmode_gpio(GPIO_TRIG, PI_OUTPUT);
    setmode_gpio(GPIO_ECHO, PI_INPUT);

    Py_RETURN_NONE;
}

static PyObject *break_measure_proc(PyObject *self, PyObject *args)
{
    if(PyArg_ParseTuple(args, "i", &b_sig))
    {
        PyErr_SetString(hcsr04Error, "measure break signal shuld be int.");
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *measure_proc(PyObject *self, PyObject *args)
{
    PulseStatus ps;

    if(gpioReg == MAP_FAILED)
    {
        PyErr_SetString(hcsr04Error, "mmap still failed " GPIOMEM ". did you call init()?");
        return NULL;
    }

    ps.EchoLevel = PI_LOW;
    gettimeofday(&(ps.EndTime), NULL);
    ps.StartTime = ps.EndTime;
    ps.StartTime.tv_sec++;

    write_gpio(GPIO_TRIG, PI_LOW);

    if(wait_level_gpio(GPIO_ECHO, PI_LOW))
    {
        PyErr_SetString(hcsr04Error, "gpio(echo) dose not become low.");
        return NULL;
    }
    trigger_gpio(GPIO_TRIG, PULSE_USEC, PI_HIGH);

    if(pulse_reader(GPIO_ECHO, &ps, &b_sig))
    {
        PyErr_SetString(hcsr04Error, "breaked at reading pulse.");
        return NULL;
    }

    return PyFloat_FromDouble(
            (double)usec_delta(&ps) * SONIC_MM_PER_USEC / 2.0
           );
}

static PyObject *close_proc(PyObject *self, PyObject *args)
{
    if(gpioReg != MAP_FAILED)
    {
        int unmap_result = munmap((void *)gpioReg, 0xB4);
        if(unmap_result != 0)
        {
            PyErr_Format(hcsr04Error, "munmap faild. gpio register still maped. %d : %s", errno, strerror(errno));
            return NULL;
        }
    }
    gpioReg = MAP_FAILED;
    Py_RETURN_NONE;
}

PyMODINIT_FUNC PyInit_rpi_hcsr04(void)
{
    PyObject *m;
    m = PyModule_Create(&module);
    if(m == NULL)
    {
        return NULL;
    }
    hcsr04Error = PyErr_NewException("hcsr04.error", NULL, NULL);
    Py_INCREF(hcsr04Error);
    PyModule_AddObject(m, "error", hcsr04Error);
    return m;
}
