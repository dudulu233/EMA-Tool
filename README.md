# EMA-Tool
open beta version 1.00

MRC(Miss Ratio Curve) plays an important role in cache locality analysis, which can be achieved by reuse distance distribution. However, obtaining the reuse distance distribution has an O(N ∗ M) complexity, where N is the total number of references in the access sequence and M is number of the unique data blocks of references. Although Recent studies have proposed various ways to decrease the computation complexity to O(N ∗ log(n)) using Search Tree, Scale Tree, Interval Tree. These methods use a balanced tree structure to get a logarithmic search time upon each reference to calculate block reuse distances. More over, SHARDS, further decreases the computation complexity with fixed amount of space. These methods still require considerable time and space overhead for processing billions of cached traces.  

EMA-Tool based on the RAR-CM, it's full name is "Efficient MRC Acquisition Tool", which can get reuse distance histogram in O(1) time complexity and very low space complexity by hash sampling and can ensure high accuracy at the same time.  
  
The input chache trace explanation and format is as follow:

'time stamp', 'cache addr'  
e.g.  
1538409600,980537001121  
1538409600,980538001121  
1538409600,327124002273  
1538409600,38225003116  
1538409601,38224003116  

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
