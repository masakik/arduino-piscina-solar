class TimerM
{

  unsigned long tmr;

  public:
    TimerM();
    bool run(unsigned long intervalo);
    bool countdown(const char action[], unsigned long tempo = 0);
};