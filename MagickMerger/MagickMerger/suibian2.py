i = input()
n = 1002
str1 = 'composite ( "C:\\Users\\zjf\\Desktop\\cc\\'
str2 = '.png" -negate ) "C:\\Users\\zjf\\Desktop\\cc\\'
str3 = '.png" -compose Copy_Opacity "C:\\Users\\zjf\\Desktop\\dd\\'
str4 = '.png"'
for x in xrange(n,i+1,2):
    print str1+str(x)+str2+str(x-1)+str3+str(x)+str(x-1)+str4