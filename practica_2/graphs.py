import csv
from telnetlib import X3PAD  # importamos csv para poder leer el fichero de entrada
import matplotlib.pyplot as plot
import pandas as pd

data1 = pd.read_csv('cyclictestURJC_RP_nort_idle.csv')
data2 = pd.read_csv('cyclictestURJC_RP_rt_idle.csv')
#data3 = pd.read_csv('cyclictestURJC_RP_nort_bonnie.csv')

df1 = pd.DataFrame(data1)
df2 = pd.DataFrame(data2)
#df3 = pd.DataFrame(data3)

X1 = list (df1.iloc[:, 2]/1000)
X2 = list (df2.iloc[:, 2]/1000)
#X3 = list (df3.iloc[:, 2]/1000)


plot.hist(X1, range=[0,55],color='r', edgecolor='black', linewidth=0.33, bins=100, alpha=0.5, label="S1:No Real-Time Idle")
plot.hist(X2, range=[0,55],color='b', edgecolor='black', linewidth=0.33, bins=100, alpha=0.5, label="S1: Real-Time Idle")
#plot.hist(X3, range=[0,55],color='g', edgecolor='black', linewidth=0.33, bins=100, alpha=0.5, label="S3: Bonnie++")

plot.title("CyclictestURJC Raspberrys")
plot.xlabel('Latencia (microsegundos')
plot.ylabel('Frecuencia')

plot.legend()
plot.show()
