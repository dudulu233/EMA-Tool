# EMA-Tool
open beta version 1.00

MRC(Miss Ratio Curve) plays an important role in cache locality analysis, which can be achieved by reuse distance distribution. However, obtaining the reuse distance distribution has an O(N ∗ M) complexity, where N is the total number of references in the access sequence and M is number of the unique data blocks of references. Although Recent studies have proposed various ways to decrease the computation complexity to O(N ∗ log(n)) using Search Tree, Scale Tree, Interval Tree. These methods use a balanced tree structure to get a logarithmic search time upon each reference to calculate block reuse distances. More over, SHARDS, further decreases the computation complexity with fixed amount of space. These methods still require considerable time and space overhead for processing billions of cached traces.  

EMA-Tool based on the RAR-CM, it's full name is "Efficient MRC Acquisition Tool", which can get reuse distance histogram in O(1) time complexity and lower space complexity than SHARDS.  
In addition, EMA-Tool incorporates the hash sampling idea of SHARDS (hash(cache_addr) mod P < T), which can reduce the space overhead to a fairly small level and ensure high accuracy at the same time.  
  
The input cache trace explanation and format is as follow:

'time stamp'(Unix time), 'cache addr'  
e.g.  
1538409600,980537001121  
1538409600,980538001121  
1538409600,327124002273  
1538409600,38225003116  
1538409601,38224003116  

We also provide some custom settings in ema.conf:  
e.g.   
cache_file=/cbs_trace1/atc_2020_trace/orignial/RAR_0  
cache_size=32  
sampling_P=1  
sampling_T=1  
io_trace_start_time=2018-10-02 0:0:0  
re_dis_start_time=2018-10-04 0:0:0  
io_trace_end_time=2018-10-05 0:0:0  
fixed_mem_size=0  
unique_ratio=36

### notice  
Because we need to build the RAR curve, and the time granularity is set to two days, the trace needs to be longer than two days at least.  
You have to make sure that the "re_dis_start_time" in ema.conf is two days after "io_trace_start_time", otherwise, no RAR curve will be available.  
  
EMA-Tool offers two use modes, fixed sampling rate and fixed memory sizes(MB)  
"fixed_mem_size=0" means mode 1, or means mode 2  
before you use the mode 2(fixed memory sizes), you have to use mode 1 to get the value of "unique_ratio", which shows the relationship between total trace and unique trace, you can get it from "log" in directory "result" after the run is over.In addition, you can set a very small sampling rate in order to run fast.

## Usage:
0` EMA-Tool use standard Linux c++ library, make sure your compiler supports it.  

1` download and edit the config file  
$vim ema.conf  
  
2`   
$make  
$./ema_tool ema.conf    
  
### Author 
designed by Dr.Yu Zhang and his assistant Siqi Luo  
### Document
[OSCA: An Online-Model Based Cache Allocation Scheme in Cloud Block Storage Systems](https://www.usenix.org/conference/atc20/presentation/zhang-yu). Zhang Y, Huang P, Zhou K, Wang H, Hu J, Ji Y, Cheng B. 2020 {USENIX} Annual Technical Conference ({USENIX} {ATC} 20). July 2020.  
### Reference
Carl A. Waldspurger, Nohhyun Park, Alexander Garthwaite,and Irfan Ahmad. Efficient MRC constructionwith SHARDS. In 13th USENIX Conference on Fileand Storage Technologies (FAST ’15), 2015.
