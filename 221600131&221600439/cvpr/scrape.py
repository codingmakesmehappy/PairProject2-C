import requests
from bs4 import BeautifulSoup
from multiprocessing import Pool, Manager
from requests.exceptions import ConnectionError
from functools import partial
from lxml import etree
import re
import time
headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36'}

def geturl(url,prelink):
    response = requests.get(url,headers=headers)
    text = response.text
    html = etree.HTML(text)
    links = html.xpath('//dt/a//@href')
    urls = []
    for link in links:
        url = prelink + link
        urls.append(url)
    return urls

def crawl(locker,data,prelink,url):
    try:
        response = requests.get(url,headers=headers)
        text = response.text
        titles,abstracts=data

        html=etree.HTML(text)
        title=html.xpath('//div[@id="papertitle"]/text()')[0]
        abstract=html.xpath('//div[@id="abstract"]/text()')[0]
        title=title[1:]
        abstract=abstract[1:]

        
        locker.acquire()  #获取锁
        titles.append(title)
        abstracts.append(abstract)
        locker.release()
    except ConnectionError:
        print('Error!',url)
    finally:
        print(url,'successed')

if __name__ == '__main__':
    start = time.clock()
    url='http://openaccess.thecvf.com/CVPR2018.py'
    prelink='http://openaccess.thecvf.com/'
    urls=geturl(url,prelink)
    
    pool = Pool(processes=32)
    manager = Manager()
    locker = manager.Lock()

    titles=manager.list()
    abstracts=manager.list()
    data = (titles,abstracts)

    scrape = partial(crawl,locker,data,prelink)  #偏函数
    pool.map(scrape,urls)

    elapsed = time.clock()-start
    print("Time used: ",elapsed)

    with open('result.txt', 'w', encoding='utf-8') as file:
        for i in range(len(titles)):
            file.write(str(i) + '\n')
            file.write('Title: '  + titles[i] + '\n')
            file.write('Abstract: ' + abstracts[i] + '\n')
            file.write('\n\n')
    