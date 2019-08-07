def ReadEncodeTrace(name):
    capture_ts_arr=[]
    encode_ts_arr=[]
    encode_len_arry=[]
    for index, line in enumerate(open(name,'r')):
        lineArr = line.strip().split()
        capture_ts_arr.append(int(lineArr[0]))
        encode_ts_arr.append(int(lineArr[1]))
        encode_len_arry.append(float(lineArr[2]))
    return capture_ts_arr,encode_ts_arr,encode_len_arry

trace_file="1280x720_encode.txt"
capture_ts_arr=[]
encode_ts_arr=[]
encode_len_arry=[]
fps=22;
gap=float(1000.0/fps)
capture_ts_arr,encode_ts_arr,encode_len_arry=ReadEncodeTrace(trace_file)
samples=len(encode_ts_arr);
#kbps
fileout="rate_compute.txt"
fout=open(fileout,'w')
for i in range(samples):
    ts=encode_ts_arr[i]
    ms=float(ts)/1000;
    if (i==0):
        rate=(encode_len_arry[i]*8/gap);
        fout.write(str(ms)+"\t")
        fout.write(str(rate)+"\n")
    else:
        delta=float(encode_ts_arr[i]-encode_ts_arr[i-1])
        rate=(encode_len_arry[i]*8/delta);
        fout.write(str(ms)+"\t")
        fout.write(str(rate)+"\n")
fout.close();