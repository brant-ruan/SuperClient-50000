f1 = open("./duqu.dat", "r")
f2 = open("./wancheng.dat", "r")
i = 1
z = f1.readlines()
w = f2.readlines()
for line in z:
	if line not in w:
		print(str(i) + " -> " + line)
		i = i + 1