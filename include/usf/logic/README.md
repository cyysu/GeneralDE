# 模块说明
    
# logic_executor详细说明
## 执行节点(logic_executor)设计原理
   logic_executor参照 *行为树* 设计的业务逻辑
    
## 执行节点(logic_executor)分类说明

### action 行为节点

    最终实现某一个具体业务的执行节点，由具体的业务实现。

### condition 条件节点

    待定义
   
### decorator 装饰节点

    装饰节点指当前节点下面包含一个子节点。
    如何执行这些节点以及如何返回执行结果是根据不同的装饰节点的类型不同而分别定义的

  - protect 保护节点
    * 执行 简单执行子节点
    * 返回值 无论子节点执行结果，返回*TRUE*
  
  - not 取反节点
    * 执行 简单执行子节点
    * 返回值 将子节点执行结果取反
   
### composite 组合节点

    组合节点指当前节点下面包涵一个执行节点的列表。
    如何调度这些执行节点、如何返回执行结果是根据执行节点的类型不同而分别定义的

  - selector 选择节点
    * 执行 按顺序执行子节点，当遇到第一个子节点返回*TRUE*，则返回*TRUE*。
    * 返回值 有一个子节点返回*TRUE*，则返回*TRUE*，否则(所有子节点执行以后，都没有返回*TRUE*)返回*FALSE*
    * 用途 优先决策队列，一般情况下，子节点是一个条件节点，整个结构就可以看成是一个 switch ... case ... 
     
  - sequence 顺序节点
    * 执行 按顺序执行子节点，当遇到第一个子节点返回*FALSE*，则返回*FALSE*。
    * 返回值 有一个子节点返回*FLASE*，则返回*FALSE*，否则(所有子节点执行以后，都没有返回*FALSE*)返回*TRUE*
    * 用途 批量执行任务
     
  - parallel 平行节点
    * 执行 执行所有的子节点，而不管兄弟节点的执行情况。节点执行的先后顺序未定义，使用时不应该有任何假设。
      当前实现上采用了按顺序一次执行的策略，后续会修改为并行执行(当某些执行是异步任务时有差别)。
    * 返回值 根据平行节点类型具体不同而不同。
      * *SUCCESS_ON_ALL* 所有子节点成功才算成功
      * *SUCCESS_ON_ONE* 只要有一个子节点成功就算成功
      * 当子节点为空时，认为执行成功
    * 用途 并行执行多个没有依赖关系的任务

## Action节点的异步执行

## 使用举例
### 通过API直接构造一个执行器

### 通过配置文件构造一个执行器

### 调用一个执行器

## 配置示例
### 配置action行为节点
1. 简单的行节点

    action1

2. 带参数的行为节点

    action2: { arg1: 1, arg2 : 2 }
    
### 配置condition条件节点

1. 完整的条件节点

    condition:
        if: action1
        do: action2
        else: action3

### 配置decorator装饰节点

1. protect装饰节点配置

    protect:
        action1
        
2. not装饰节点配置

    not:
        action1

### 配置composite组合节点

1. 配置选择节点

    selector:
        - condition:
            if: action1
            do: action2
        - condition:
            if: action3
            do: action4

1. 配置sequence顺序节点

    sequence:
        - action1
        - action2
        - action3
        

1. 配置sequence顺序节点的简略方法

    - action1
    - action2
    - action3
    
1. 配置parallel平行节点的完整方法

    parallel:
        policy: SUCCESS_ON_ALL   #或者 SUCCESS_ON_ONE，默认为SUCCESS_ON_ALL
        childs:
            - action1
            - action2


1. 配置parallel平行节点的简略方法

    parallel:  #policy默认为SUCCESS_ON_ALL
        - action1
        - action2
