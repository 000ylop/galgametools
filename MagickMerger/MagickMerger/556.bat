composite ( "C:\Users\zjf\Desktop\cc\102.png" -negate ) "C:\Users\zjf\Desktop\cc\101.png" -compose Copy_Opacity "C:\Users\zjf\Desktop\dd\101102.png"
composite ( "C:\Users\zjf\Desktop\cc\104.png" -negate ) "C:\Users\zjf\Desktop\cc\103.png" -compose Copy_Opacity "C:\Users\zjf\Desktop\dd\103104.png"

i = inupt(input the number:)
n = 101
for x in xrange(0,i-101,2):
    str = r'composite ( "C:\Users\zjf\Desktop\cc\%d.png" -negate ) "C:\Users\zjf\Desktop\cc\%d.png" -compose Copy_Opacity "C:\Users\zjf\Desktop\dd\%s.png"'%(n,n+1,str(n)+str(n+1))
    print str

