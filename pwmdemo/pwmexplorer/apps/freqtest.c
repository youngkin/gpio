//
// Build - gcc -o freqtest freqtest.c  -lwiringPi -lpthread
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
//    flag.StringVar(&pwmType, "pwmType", "hardware", "defines whether software or hardware PWM should be used")
//    flag.StringVar(&divisorStr, "div", "9600000", "PWM clock frequency divisor")
//    flag.StringVar(&cycleStr, "cycle", "2400000", "PWM cycle/period length in microseconds")
//    flag.StringVar(&pwmPinStr, "pin", "18", "GPIO PWM pin")
//    flag.StringVar(&pulseWidthStr, "pulseWidth", "4", "PWM Pulse Width")
//    flag.Parse()
//    fmt.Printf("Input: PWM pin: %s, PWM Type: %s, divisor: %s, cycle: %s, pulse width: %s\n", pwmPinStr, pwmType, divisorStr, cycleStr, pulseWidthStr)
//
//    divisor, cycle, pin, pulse := getParms(divisorStr, cycleStr, pwmPinStr, pulseWidthStr)
//    fmt.Printf("Using: PWM pin: %s, PWM Type: %s, divisor: %s, cycle: %s, pulse width: %s\n", pwmPinStr, pwmType, divisorStr, cycleStr, pulseWidthStr)

    char *pwmType = NULL;
    char *pwmMode = NULL;
    int divisor = 0;
    int cycle = 0;
    int pin = 0;
    int pulseWidth = 0;
                                                                     
    int c;

    while (1)
    {
        static struct option long_options[] =
            {
                {"type",        optional_argument, 0, 't'},
                {"mode",        optional_argument, 0, 'm'},
                {"divisor",     optional_argument, 0, 'd'},
                {"cycle",       optional_argument, 0, 'c'},
                {"pin",         optional_argument, 0, 'p'},
                {"pulseWidth",  optional_argument, 0, 'w'},
                 {0, 0, 0, 0}
            };
            /* getopt_long stores the option index here. */
            int option_index = 0;
                   
            c = getopt_long (argc, argv, "t::m::d::c::p::w::",
                             long_options, &option_index);
                         
            /* Detect the end of the options. */
            if (c == -1)
                break;
                               
            switch (c)
             {
             case 0:
                 /* If this option set a flag, do nothing else now. */
                 if (long_options[option_index].flag != 0)
                     break;
                 printf ("option %s", long_options[option_index].name);
                 if (optarg)
                     printf (" with arg %s", optarg);
                     printf ("\n");
                     break;

             case 't':
                 printf ("option -t with value '%s'\n", optarg);
                 break;

             case 'm':
                printf ("option -m with value '%s'\n", optarg);
                break;

              case 'd':
                printf ("option -d with value `%s'\n", optarg);
                    break;
   
              case 'c':
                  printf ("option -c with value `%s'\n", optarg);
                   break;
                                               
              case 'p':
                printf ("option -p with value `%s'\n", optarg);
                    break;
                           
              case 'w':
                printf ("option -w with value `%s'\n", optarg);
                    break;
                           
              case '?':
                 /* getopt_long already printed an error message. */
                 break;
                 
              default:
                abort ();
              }
    }
    /* Print any remaining command line arguments (not options). */
    if (optind < argc)
    {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
            putchar ('\n');
                            
    }

  exit (0);
}

