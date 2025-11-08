# motor_speed_regulation

我们要实现的功能  
1. 自动电机启动  
2. 手动点击启动  
3. 启动后的速度控制

我们的电机有两种调压：
1. pwm调压
2. dc调压

我们优先实现pwm调压。


原理：
    我们读取滑动变阻器的电压值，与pwm对应，实现手动调速。
    

ads1115函数执行过程：
1. 配置IIC总线              `iic_init`
2. 将ads1115挂载到IIC总线   `ads1115_init()`
3. 配置ads1115寄存器        `ads1115_config(ads1115_config_t *ads1115_config);`
4. 将配置数据写入寄存器       
5. 持续读写                 