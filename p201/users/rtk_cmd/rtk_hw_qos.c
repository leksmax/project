#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#ifdef KERNEL_2_6_30
#include <linux/config.h>
#endif
#ifdef KERNEL_3_10
#include "../../linux-3.10/include/generated/autoconf.h"
#endif
#include <linux/netlink.h>
#include <net/rtl/rtl_types.h>

#if defined(CONFIG_RTL_8197F)
#ifdef KERNEL_3_10
#include "../../linux-3.10/drivers/net/rtl819x/common/rtl865x_vlan.h"
#endif
#endif

#include <linux/netdevice.h>
#include "rtk_cmd.h"
#include "rtk_hw_qos.h"

//#define QOS_DEBUG 1

#if defined(QOS_DEBUG)
#define RTK_QOS_PRINTF(format, args...) printf(format, ## args)
#else
#define RTK_QOS_PRINTF(format, args...)
#endif

/*rtk_cmd qos Priority_Assign Port_Based port0 3 port1 4 port2 5
dw bb804714
*/
static inline int rtk_port_based_priority_assign(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, temp;
	int ret = 0;

	if((_param[3] != NULL) && (!(memcmp(_param[3],"Port_Based",strlen(_param[3]))))){
			send_data->action=PORT_BASED_PRIORITY_ASSIGN;
			for(i=4; i<_num; i++)
			{
				if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
					temp = _param[i][4] - '0';
					if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8')){
						send_data->qos_data.port_based_priority.port[temp] = *(_param[i+1]) - '0';
						send_data->qos_data.port_based_priority.portmask |= 1 << temp;
						ret += 1;
						RTK_QOS_PRINTF("Priority %d is good for port[%d]\n", send_data->qos_data.port_based_priority.port[temp], temp);
					}else{
							goto QOS_ERROR;
					}
				}
			}
			RTK_QOS_PRINTF("Portmask is %x\n", send_data->qos_data.port_based_priority.portmask);
		}
	return ret;
QOS_ERROR:
	return 0;
}


/*rtk_cmd qos Priority_Assign Vlan_Based vlan0 0 vlan1 1 vlan2 2 vlan3 3 vlan4 4 vlan5 5 vlan6 6 vlan7 7
dw bb804730
*/
static inline int rtk_vlan_based_priority_assign(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, temp;
	int ret = 0;

	if((_param[3] != NULL) && (!(memcmp(_param[3],"Vlan_Based",strlen(_param[3]))))){
			send_data->action = VLAN_BASED_PRIORITY_ASSIGN;
			for(i=4; i<_num; i++)
			{
				if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i], "vlan", (strlen(_param[i]) -1))))){
					temp = _param[i][4] - '0';
					if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8')){
						send_data->qos_data.vlan_based_priority.vlan[temp] = *(_param[i+1]) - '0';
						send_data->qos_data.vlan_based_priority.vlanmask |= 1 << temp;
						ret += 1;
						RTK_QOS_PRINTF("Priority %d is good for vlan[%d]\n", send_data->qos_data.vlan_based_priority.vlan[temp], temp);
					}else{
							goto QOS_ERROR;
					}
				}
			}
		}
	return ret;
QOS_ERROR:
	return 0;
}

#if defined(CONFIG_RTL_8197F)
static inline int rtk_vid_based_priority_assin(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i=0, temp=0, index=0;
	int ret = 0;

	if(_num > 20)		/*we define input 8 vids tops: 4+2*8*/
		goto QOS_ERROR;

	if((_param[3] != NULL) && (!(memcmp(_param[3],"Vid_Based",strlen(_param[3]))))){
		send_data->action = VID_BASED_PRIORITY_ASSIGN;
		for(i=4; i<_num; i++)
		{
			//temp = _param[i][3];
			if((_param[i] != NULL) && (strlen(_param[i]) >= 4) && (!(memcmp(_param[i], "vid", 3))) && (index < 8)){				
				if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8')){
					const char *p = _param[i] + 3;
					char str_tmp[16] = {0};
					
					memcpy(str_tmp, p, (strlen(_param[i])-3));
					temp = strtol(str_tmp, NULL, 10);

					send_data->qos_data.vid_based_priority.vid[index] = temp;
					send_data->qos_data.vid_based_priority.pri[index] = *(_param[i+1]) - '0';	
					
					ret += 1;
					RTK_QOS_PRINTF("Priority 0x%x is good for vid[0x%x]\n", send_data->qos_data.vid_based_priority.pri[index], send_data->qos_data.vid_based_priority.vid[index]);
					index++;
					
				}else{
		 			goto QOS_ERROR;
				}
			}

		}
	}
	return ret;
QOS_ERROR:
	return 0;	
}
#endif



/*rtk_cmd qos Priority_Assign Dscp_Based dscp0 1
dw bb804734
*/
static inline int rtk_dscp_based_priority_assign(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, temp;
	int ret = 0;

	 if((_param[3] != NULL) && (!(memcmp(_param[3],"Dscp_Based",strlen(_param[3]))))){
			send_data->action = DSCP_BASED_PRIORITY_ASSIGN;
			for(i=4; i<_num; i++)
			{
				if((_param[i] != NULL) && (((strlen(_param[i]) == 5) && (!(memcmp(_param[i], "dscp", (strlen(_param[i]) -1))))) ||
				  ((strlen(_param[i]) == 6) && (!(memcmp(_param[i], "dscp", (strlen(_param[i]) -2))))))){
					if(strlen(_param[i]) == 6)
						temp = (_param[i][4] - '0')*10 + (_param[i][5] - '0');
					else
						temp = _param[i][4] - '0';

					if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8')){
						send_data->qos_data.dscp_based_priority.dscp[temp] = *(_param[i+1]) - '0';
						if(temp < 32)
							send_data->qos_data.dscp_based_priority.dscpmask1 |= 1 << temp;
						else
							send_data->qos_data.dscp_based_priority.dscpmask2 |= 1 << (temp-32);
						ret += 1;
						RTK_QOS_PRINTF("Priority %d is good for dscp[%d]\n", send_data->qos_data.dscp_based_priority.dscp[temp], temp);
					}else{
							goto QOS_ERROR;
					}
				}
			}
		}
	return ret;
 QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Queue_Num port0 1
dw bb804754
*/
static inline int rtk_queue_number(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, temp;
	int ret = 0;

	send_data->action = QUEUE_NUMBER;
	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
			temp = _param[i][4] - '0';
			#if defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8198C)
			if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) > '0') && (*(_param[i+1]) < '9'))
			#else
			if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) > '0') && (*(_param[i+1]) < '7'))
			#endif
			{
				send_data->qos_data.queue_num.port[temp] = *(_param[i+1]) - '0';
				send_data->qos_data.queue_num.portmask |= 1 << temp;
				ret += 1;
				RTK_QOS_PRINTF("Queue number %d is good for vlan[%d]\n", send_data->qos_data.queue_num.port[temp], temp);
			}else{
					goto QOS_ERROR;
			}
		}
	}
	return ret;
QOS_ERROR:
	return 0;

}

/*users can config 6 queues type for a port once a time
example:
rtk_cmd qos Queue_Type STRICT port0 q0 q1 q2 q3 q4 q5
dw bb8048b4  --> port0
*/
static inline int rtk_queue_type_strict(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, temp_port, temp_queue;
	int ret = 0;

	if((_param[3] != NULL) && (!(memcmp(_param[3],"STRICT",strlen(_param[3]))))){
		send_data->action = QUEUE_TYPE_STRICT;
		for(i=4; i<_num; i++)
		{
			if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
				temp_port = _param[i][4] - '0';
				for(j=i+1; j<_num; j++)
				{
					if((_param[j] != NULL) && (strlen(_param[j]) == 2) && (!(memcmp(_param[j],"q",(strlen(_param[j])-1))))){
						temp_queue = _param[j][1] - '0';
						send_data->qos_data.queue_type.queue[temp_port][temp_queue] = 255;
						send_data->qos_data.queue_type.portmask |= 1<<temp_port;
						send_data->qos_data.queue_type.queuemask |= 1<<temp_queue;
						ret += 1;
					}else{
						goto QOS_ERROR;
					}
				}
			}
		}
	}
	return ret;
QOS_ERROR:
	return 0;
}

/*	rtk_cmd qos Queue_Flow_Control port4 q0 on q1 off q4 on
	dw bb8045d0 - bb8045d4
*/

#if defined(SET_QUEUE_FLOW_CONTROL)
static inline int rtk_queue_flow_control(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	unsigned int i, j, temp_port, temp_queue, status;
	int ret = 0;

	send_data->action = QUEUE_FLOW_CONTROL;
	for(i=3; i<_num; i++){
		if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
			temp_port = _param[i][4] - '0';

			for(j=i+1; j<_num; j++)
			{
				if((_param[j] != NULL) && (strlen(_param[j]) == 2) && (!(memcmp(_param[j],"q",(strlen(_param[j])-1))))){
					temp_queue = _param[j][1] - '0';

					if(_param[j+1] != NULL){
						if(strncmp(_param[j+1], "on", 2) == 0){
							status = 1;
						}
						else if(strncmp(_param[j+1], "off", 2) == 0){
							status = 0;
						}
						else{
							goto QOS_ERROR;
						}

						send_data->qos_data.queue_flow_control.portmask |= 1<<temp_port;
						send_data->qos_data.queue_flow_control.queuemask |= 1<<temp_queue;
						send_data->qos_data.queue_flow_control.queue[temp_port][temp_queue] = status;
						ret += 1;
					}
					else{
						goto QOS_ERROR;
					}
				}
			}
		}
	}
	
	return ret;
QOS_ERROR:
	return 0;
}
#endif

/*users can config 6 queues type for a port once a time
example:
rtk_cmd qos Queue_Type WEIGHTED port0 q0 5 q1 16 q2 17 q3 18 q4 19 q5 20
dw bb8048b4  --> port0
*/
static inline int rtk_queue_type_weighted(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, temp_port, temp_queue, temp;
	int ret = 0;

	if((_param[3] != NULL) && (!(memcmp(_param[3],"WEIGHTED",strlen(_param[3]))))){
		send_data->action = QUEUE_TYPE_WEIGHTED;
		for(i=4; i<_num; i++)
		{
			if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
				temp_port = _param[i][4] - '0';
				for(j=i+1; j<_num; j++)
				{
					if((_param[j] != NULL) && (strlen(_param[j]) == 2) && (!(memcmp(_param[j],"q",(strlen(_param[j])-1))))){
						temp_queue = _param[j][1] - '0';
						if(_param[j+1] != NULL){
							if(strlen(_param[j+1])== 1)
								temp = *(_param[j+1]) - '0';
							else if(strlen(_param[j+1])== 2)
								temp = (_param[j+1][0] - '0')*10 + (_param[j+1][1] - '0');
							else if(strlen(_param[j+1])== 3)
								temp = (_param[j+1][0] - '0')*100 + (_param[j+1][1] - '0')*10 + (_param[j+1][2] - '0');

							if(temp > 127)
								goto QOS_ERROR;

							send_data->qos_data.queue_type.queue[temp_port][temp_queue] = temp;
							send_data->qos_data.queue_type.portmask |= 1<<temp_port;
							send_data->qos_data.queue_type.queuemask |= 1<<temp_queue;
							ret += 1;
						}else{
							goto QOS_ERROR;
						}
					}
				}
			}
		}
	}
	return ret;
QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Priority_to_Qid pri0 0 pri1 1 pri3 3 pri4 4 pri5 5 pri6 5 pri7 5
dw bb804718
*/
static inline int rtk_priority_to_qid(int _num, char* _param[], struct qos_cmd_info_s * send_data)
{
	int i, temp;
	int ret = 0;

	send_data->action = PRIORITY_TO_QID;

	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 4) && (!(memcmp(_param[i],"pri",(strlen(_param[i])-1))))){
			temp = _param[i][3] - '0';
			#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8'))
			#else
			if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '6'))
			#endif
			{
				send_data->qos_data.sys_priority.priority[temp] = *(_param[i+1]) - '0';
				send_data->qos_data.sys_priority.prioritymask |= 1 << temp;
				ret += 1;
				RTK_QOS_PRINTF("Qid %d is good for priority[%d]\n", send_data->qos_data.port_based_priority.port[temp], temp);
			}else{
					goto QOS_ERROR;
			}
		}
	}
	return ret;
QOS_ERROR:
	return 0;
}

/*rtk_cmd qos CPU_Priority_to_Qid pri0 0 pri1 1 pri3 3 pri4 4 pri5 5 pri6 5 pri7 5
dw bb804758
*/
static inline int rtk_cpu_priority_to_qid(int _num, char* _param[], struct qos_cmd_info_s * send_data)
{
	int i, temp;
	int ret = 0;

	send_data->action = CPU_PRIORITY_TO_QID;

	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 4) && (!(memcmp(_param[i],"pri",(strlen(_param[i])-1))))){
			temp = _param[i][3] - '0';
			if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '6')){
				send_data->qos_data.cpu_priority.priority[temp] = *(_param[i+1]) - '0';
				send_data->qos_data.cpu_priority.prioritymask |= 1 << temp;
				ret += 1;
				RTK_QOS_PRINTF("CPU Qid %d is good for priority[%d]\n", send_data->qos_data.cpu_priority.priority[temp], temp);
			}else{
					goto QOS_ERROR;
			}
		}
	}
	return ret;
QOS_ERROR:
	return 0;
}

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
/*	rtk_cmd qos Mark_to_Priority netif eth1 mark10 2 mark1 7	
	rtk_cmd qos Mark_to_Priority netif eth1 flush	
*/
static inline int rtk_mark_to_priority(int _num, char* _param[], struct qos_cmd_info_s * send_data)
{
	int i, j;
	int mark, prio, ret = 0;

	send_data->action = MARK_TO_PRIORITY;

	if((_param[3] != NULL) && (strlen(_param[3]) == 5) 
	&& !(memcmp(_param[3], "netif", (strlen(_param[3])))) && (_param[4] != NULL)){
		snprintf(send_data->qos_data.mark_to_prio.netifName, sizeof(send_data->qos_data.mark_to_prio.netifName),
			"%s", _param[4]);

		if((_param[5] != NULL) && (strlen(_param[5]) == 5) && 
		!(memcmp(_param[5], "flush", (strlen(_param[5]))))){
			for(j = 0; j < MAX_MARK_NUM_PER_DEV; j++){
				send_data->qos_data.mark_to_prio.markInfo[j].delflag = 1;
				send_data->qos_data.mark_to_prio.countmask |= (1 << j);
				ret += 1;
			}
		}
		else{
			for(i = 5; i < _num; i = i+2){
				if((_param[i] != NULL) && _param[i+1] != NULL){
					char *substr = strtok(_param[i], "mark");
					if(substr != NULL){
						mark = atoi(substr);
						prio = *(_param[i+1])-'0';
						
						if(mark && prio > -1 && prio < 8){
							for(j = 0; j < MAX_MARK_NUM_PER_DEV; j++){
								if(send_data->qos_data.mark_to_prio.markInfo[j].mark == mark){	// find a match
									send_data->qos_data.mark_to_prio.markInfo[j].priority = prio;
									send_data->qos_data.mark_to_prio.countmask |= (1 << j);
									ret += 1;
									break;
								}
							}
								
							if(j == MAX_MARK_NUM_PER_DEV){	// need a new entry
								for(j = 0; j < MAX_MARK_NUM_PER_DEV; j++){
									if(send_data->qos_data.mark_to_prio.markInfo[j].mark == 0){	// means entry not used
										send_data->qos_data.mark_to_prio.markInfo[j].mark = mark;
										send_data->qos_data.mark_to_prio.markInfo[j].priority = prio;
										send_data->qos_data.mark_to_prio.countmask |= (1 << j);
										ret += 1;
										break;
									}
								}
							
								if(j == MAX_MARK_NUM_PER_DEV){
									printf("Netif %s Mark to Priority Mapping Full.\n ", send_data->qos_data.mark_to_prio.netifName);
									goto QOS_ERROR;
								}
							}
						}
						else{
							goto QOS_ERROR;
						}
					}
					else{
						goto QOS_ERROR;
					}	
				}
			}
		}
	}
		
	return ret;
	
QOS_ERROR:
	return 0;	
}
#endif

/*rtk_cmd qos Priority_Decision port 15 vlan 1 dscp 2 acl 13  nat 14
dw bb804750
*/
static inline int rtk_priority_decision(int _num, char* _param[], struct qos_cmd_info_s * send_data)
{
	int i, temp;
	int ret = 0;

	send_data->action = PRIORITY_DECISION;
	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 4) && (!(memcmp(_param[i], "port", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');

				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[0] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		else if((_param[i] != NULL) && (strlen(_param[i]) == 4) && (!(memcmp(_param[i], "vlan", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');

				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[1] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		else if((_param[i] != NULL) && (strlen(_param[i]) == 4) && (!(memcmp(_param[i], "dscp", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');

				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[2] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		else if((_param[i] != NULL) && (strlen(_param[i]) == 3) && (!(memcmp(_param[i], "acl", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');

				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[3] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		else if((_param[i] != NULL) && (strlen(_param[i]) == 3) && (!(memcmp(_param[i], "nat", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');

				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[4] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		#if defined(CONFIG_RTL_8197F)
		else if((_param[i] != NULL) && (strlen(_param[i]) == 3) && (!(memcmp(_param[i], "vid", (strlen(_param[i])))))){
			if((_param[i+1] != NULL) && ((strlen(_param[i+1]) == 1) ||(strlen(_param[i+1]) == 2))){
				if((strlen(_param[i+1]) == 1))
					temp = *(_param[i+1]) - '0';
				else
					temp = (_param[i+1][0] - '0')*10 + (_param[i+1][1] - '0');
				if(temp > 15)
					goto QOS_ERROR;
				send_data->qos_data.pri_decision.decision[5] = temp;
				ret += 1;
			}else{
					goto QOS_ERROR;
			}
		}
		#endif
	}

	return ret;
QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Remark VLAN port0 pri0 5 pri1 6
dw bb80476c
*/
static inline int rtk_vlan_remark(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, temp_port, temp_pri;
	int ret = 0;

	if((_param[3] != NULL) && (strlen(_param[3]) == 4)&& (!(memcmp(_param[3],"VLAN",strlen(_param[3]))))){
		send_data->action = VLAN_REMARK;
		for(i=4; i<_num; i++)
		{
			if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
				temp_port = _param[i][4] - '0';
				for(j=i+1; j<_num; j++)
				{
					if((_param[j] != NULL) && (strlen(_param[j]) == 4) && (!(memcmp(_param[j],"pri",(strlen(_param[j])-1))))){
						temp_pri = _param[j][3] - '0';
						if((_param[j+1] != NULL) && (strlen(_param[j+1]) == 1) && (*(_param[j+1]) >= '0') && (*(_param[j+1]) < '8')){
							send_data->qos_data.vlan_remark.remark[temp_port][temp_pri] = *(_param[j+1]) - '0';
							send_data->qos_data.vlan_remark.portmask |= 1<<temp_port;
							send_data->qos_data.vlan_remark.prioritymask |= 1<<temp_pri;
							ret += 1;
						}else{
								goto QOS_ERROR;
						}
					}
				}
			}
		}

	}
	return ret;
QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Remark DSCP port0 pri0 5 pri1 60
dw bb804770
*/
static inline int rtk_dscp_remark(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, temp_port, temp_pri, temp;
	int ret = 0;

	if((_param[3] != NULL) && (strlen(_param[3]) == 4)&& (!(memcmp(_param[3],"DSCP",strlen(_param[3]))))){
		send_data->action = DSCP_REMARK;
		for(i=4; i<_num; i++)
		{
			if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
				temp_port = _param[i][4] - '0';
				for(j=i+1; j<_num; j++)
				{
					if((_param[j] != NULL) && (strlen(_param[j]) == 4) && (!(memcmp(_param[j],"pri",(strlen(_param[j])-1))))){
						temp_pri = _param[j][3] - '0';
						if((_param[j+1] != NULL) && ((strlen(_param[j+1]) == 1) || (strlen(_param[j+1]) == 2))){
							if(strlen(_param[j+1]) == 1)
								temp = *(_param[j+1]) - '0';
							else
								temp = (_param[j+1][0] - '0')*10 + (_param[j+1][1] - '0');

							if(temp > 63)
								goto QOS_ERROR;

							send_data->qos_data.dscp_remark.remark[temp_port][temp_pri] = temp;
							send_data->qos_data.dscp_remark.portmask |= 1<<temp_port;
							send_data->qos_data.dscp_remark.prioritymask |= 1<<temp_pri;
							ret += 1;
							//printf("port[%d], priority[%d], ret is %d, remark is %d\n", send_data->qos_data.dscp_remark.portmask,
								//send_data->qos_data.dscp_remark.prioritymask, ret, send_data->qos_data.dscp_remark.remark[temp_port][temp_pri] );
						}else{
								goto QOS_ERROR;
						}
					}
				}
			}
		}

	}

	return ret;
QOS_ERROR:
	return 0;
}

/*
rtk_cmd qos Port_Rate port0 apr 1566
queue rate = (1566+1)*64kbps
0xbb8048b0: WFQRCRP0
*/
static inline int rtk_port_rate(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, k, temp_port, temp_queue;
	int temp_apr = 0;
	#if defined(CONFIG_RTL_8197F)
	int coefficient[6] = {100000, 10000, 1000, 100, 10, 1};
	#else
	int coefficient[5] = {10000, 1000, 100, 10, 1};
	#endif
	int ret = 0;

	send_data->action = PORT_RATE;
	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
			temp_port = _param[i][4] - '0';
			for(j=i+1; j<_num; j++)
			{
				if((_param[j] != NULL) && (strlen(_param[j]) == 3) && (!(memcmp(_param[j],"apr",(strlen(_param[j])))))){
					#if defined(CONFIG_RTL_8197F)
					if((_param[j+1] != NULL) && (strlen(_param[j+1]) > 0) &&((strlen(_param[j+1]) < 7)))
					#else
					if((_param[j+1] != NULL) && (strlen(_param[j+1]) > 0) &&((strlen(_param[j+1]) < 6)))
					#endif
					{
						for(k=strlen(_param[j+1]); k>0; k--)
						{
							#if defined(CONFIG_RTL_8197F)
							temp_apr += (_param[j+1][k-1] - '0')*coefficient[(k-1)+(6- strlen(_param[j+1]))];
							#else
							temp_apr += (_param[j+1][k-1] - '0')*coefficient[(k-1)+(5- strlen(_param[j+1]))];
							#endif
						}

						#if defined(CONFIG_RTL_8197F)
						if(temp_apr > 0xfffff){
							goto QOS_ERROR;
						}
						#else
						if(temp_apr > 0x3fff){
							goto QOS_ERROR;
						}
						#endif

						send_data->qos_data.port_rate.apr[temp_port] = temp_apr;
						send_data->qos_data.port_rate.portmask |= 1<<temp_port;
						ret += 1;
					}else{
						goto QOS_ERROR;
					}
				}
			}
		}
	}

	return ret;
QOS_ERROR:
	return 0;
}

/*
rtk_cmd qos Rate port0 q0 ppr 0 burst 255 apr 1525
queue rate = (1525+1)*64kbps
0xbb804800: P0Q0RGCR
*/
static inline int rtk_queue_rate(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, j, k, temp_port, temp_queue;
	int temp_apr = 0;
	int temp_burst = 0;
	#if defined(CONFIG_RTL_8197F)
	int coefficient[6] = {100000, 10000, 1000, 100, 10, 1};
	#else
	int coefficient[5] = {10000, 1000, 100, 10, 1};
	#endif
	int ret = 0;

	send_data->action = QUEUE_RATE;
	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (strlen(_param[i]) == 5) && (!(memcmp(_param[i],"port",(strlen(_param[i])-1))))){
			temp_port = _param[i][4] - '0';
			for(j=i+1; j<_num; j++)
			{
				if((_param[j] != NULL) && (strlen(_param[j]) == 2) && (!(memcmp(_param[j],"q",(strlen(_param[j])-1))))){
						temp_queue = _param[j][1] - '0';
						if((_param[j+1] != NULL) && (strlen(_param[j+1]) == 3) && (!(memcmp(_param[j+1],"ppr",(strlen(_param[j+1])))))){
								if((_param[j+2] != NULL) && (strlen(_param[j+2]) == 1) && (*(_param[j+2]) >= '0')&& (*(_param[j+2]) <'8')){
										send_data->qos_data.queue_rate.ppr[temp_port][temp_queue] = *(_param[j+2]) - '0';

								}else{
										goto QOS_ERROR;
								}

								if((_param[j+3] != NULL) && (strlen(_param[j+3]) == 5) && (!(memcmp(_param[j+3],"burst",(strlen(_param[j+3])))))){
									if((_param[j+4] != NULL) && (strlen(_param[j+4]) > 0) &&((strlen(_param[j+4]) < 4))){
										for(k=strlen(_param[j+4]); k>0; k--)
										{
											#if defined(CONFIG_RTL_8197F)
											temp_burst += (_param[j+4][k-1] - '0')*coefficient[(k-1)+(6- strlen(_param[j+4]))];
											#else
												temp_burst += (_param[j+4][k-1] - '0')*coefficient[(k-1)+(5- strlen(_param[j+4]))];
											#endif
										}

										if(temp_burst > 255){

											goto QOS_ERROR;

										}

										send_data->qos_data.queue_rate.burst[temp_port][temp_queue] = temp_burst;
									}else{
										goto QOS_ERROR;
									}

									if((_param[j+5] != NULL) && (strlen(_param[j+5]) == 3) && (!(memcmp(_param[j+5],"apr",(strlen(_param[j+5])))))){
										if((_param[j+6] != NULL) && (strlen(_param[j+6]) > 0) &&((strlen(_param[j+6]) < 7))){
											for(k=strlen(_param[j+6]); k>0; k--)
											{
												#if defined(CONFIG_RTL_8197F)
												temp_apr += (_param[j+6][k-1] - '0')*coefficient[(k-1)+(6- strlen(_param[j+6]))];
												#else
												temp_apr += (_param[j+6][k-1] - '0')*coefficient[(k-1)+(5- strlen(_param[j+6]))];
												#endif
											}

											#if defined (CONFIG_RTL_8197F)
											if(temp_apr > 0xfffff){
												goto QOS_ERROR;
											}
											#else
											if(temp_apr > 0x3fff){
												goto QOS_ERROR;
											}
											#endif

											send_data->qos_data.queue_rate.apr[temp_port][temp_queue] = temp_apr;
											send_data->qos_data.queue_rate.portmask |= 1<<temp_port;
											send_data->qos_data.queue_rate.queuemask |= 1<<temp_queue;
											ret += 1;
										}else{
												goto QOS_ERROR;
										}
									}
								}

						}
					}
				}
			}
		}

	return ret;
QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Flow_Control ENABLE
dw bb8045d0
*/
static inline int rtk_flow_control_config(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	if((_param[3] != NULL) && (!(memcmp(_param[3],"ENABLE",strlen(_param[3]))))){
		send_data->action=FLOW_CONTROL_ENABLE;
		return 1;
	}else if((_param[3] != NULL) && (!(memcmp(_param[3],"DISABLE",strlen(_param[3]))))){
		send_data->action=FLOW_CONTROL_DISABLE;
		return 1;
	}else{
		return 0;
	}
}

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
/*rtk_cmd qos TC_COMMAND DISABLE */
static inline int rtk_tc_command_config(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	if((_param[3] != NULL) && (!(memcmp(_param[3],"ENABLE",strlen(_param[3]))))){
		send_data->action=TC_COMMAND_ENABLE;
		return 1;
	}else if((_param[3] != NULL) && (!(memcmp(_param[3],"DISABLE",strlen(_param[3]))))){
		send_data->action=TC_COMMAND_DISABLE;
		return 1;
	}else{
		return 0;
	}
}
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static inline void strict_queue_show(unsigned char queue[8][8])
#else
static inline void strict_queue_show(unsigned char queue[8][6])
#endif
{
	int i, j;

	for(i=0; i<8; i++){
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		for(j=0; j<8; j++)
		#else
		for(j=0; j<6; j++)
		#endif
		{
			if(queue[i][j] == 255)
				RTK_QOS_PRINTF("Queue[%d] of port[%d] is configured as STRICT type!\n", j, i);
		}
	}
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static inline void weighted_queue_show(unsigned char queue[8][8])
#else
static inline void weighted_queue_show(unsigned char queue[8][6])
#endif
{
	int i, j;

	for(i=0; i<8; i++){
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		for(j=0; j<8; j++)
		#else
		for(j=0; j<6; j++)
		#endif
		{
			if((queue[i][j]>0) && (queue[i][j]!= 255))
				RTK_QOS_PRINTF("Queue[%d] of port[%d] is configured as WEIGHTED type, and weight is %u!\n", j, i, queue[i][j]);
		}
	}
}

#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
/*rtk_cmd qos Dscpv6_Priority_Assign  dscpv60 1
dw bb805214 7
*/
static inline int rtk_dscpv6_based_priority_assign(int _num, char* _param[], struct qos_cmd_info_s* send_data)
{
	int i, temp;
	int ret = 0;

	send_data->action = DSCPV6_BASED_PRIORITY_ASSIGN;
	for(i=3; i<_num; i++)
	{
		if((_param[i] != NULL) && (((strlen(_param[i]) == 7) && (!(memcmp(_param[i], "dscpv6", (strlen(_param[i]) -1))))) ||
			((strlen(_param[i]) == 8) && (!(memcmp(_param[i], "dscpv6", (strlen(_param[i]) -2))))))){
				if(strlen(_param[i]) == 8)
					temp = (_param[i][6] - '0')*10 + (_param[i][7] - '0');
				else
					temp = _param[i][6] - '0';

				if(temp>63 || temp<0)
					goto QOS_ERROR;

				if((_param[i+1] != NULL) && (strlen(_param[i+1]) == 1) && (*(_param[i+1]) >= '0') && (*(_param[i+1]) < '8')){
					send_data->qos_data.dscpv6_based_priority.dscpv6[temp] = *(_param[i+1]) - '0';
					if(temp < 32)
						send_data->qos_data.dscpv6_based_priority.dscpv6mask1 |= 1 << temp;
					else
						send_data->qos_data.dscpv6_based_priority.dscpv6mask2 |= 1 << (temp-32);
					ret += 1;
					RTK_QOS_PRINTF("Priority %d is good for dscpv6[%d]\n", send_data->qos_data.dscpv6_based_priority.dscpv6[temp], temp);
				}else{
						goto QOS_ERROR;
				}
			}
	}
	return ret;
 QOS_ERROR:
	return 0;
}

/*rtk_cmd qos Dscpv6_Priority_Decision 15
dw bb805204
*/
static inline int rtk_dscpv6_priority_decision(int _num, char* _param[], struct qos_cmd_info_s * send_data)
{
	int temp;
	send_data->action=DSCPV6_PRIORITY_DECISION;
	if(_param[3] != NULL && strlen(_param[3]) == 2)
		temp = (_param[3][0] - '0')*10 + (_param[3][1] - '0');
	else if(_param[3] != NULL && strlen(_param[3]) == 1)
		temp = (_param[3][0] - '0');
	else 
		goto QOS_ERROR;

	if(temp>=0 && temp<=15)
	{
		send_data->qos_data.dscpv6_pri_decision.dscpv6_decision = temp;
		RTK_QOS_PRINTF("Ipv6 Dscp based Priority weight is %d\n", send_data->qos_data.dscpv6_pri_decision.dscpv6_decision);
	}
	else
		goto QOS_ERROR; 

	return 1;
QOS_ERROR:
	return 0;
}

#endif
int rtk_hw_qos_parse(int _num, char* _param[])
{
	struct qos_cmd_info_s send_data, recv_data;
	struct nl_data_info send_info,recv_info;

	memset(&send_data, 0, sizeof(struct qos_cmd_info_s));

	if((_param[2] != NULL) && (!(memcmp(_param[2],"Priority_Assign",strlen(_param[2])))))
	{
		if(rtk_port_based_priority_assign(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Port based priority assign ok!\n");
		}else if(rtk_vlan_based_priority_assign(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Vlan based priority assign ok!\n");
		}else if(rtk_dscp_based_priority_assign(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Dscp based priority assign ok!\n");
		}
		#if defined(CONFIG_RTL_8197F)
		else if(rtk_vid_based_priority_assin(_num, _param, &send_data) > 0){
					RTK_QOS_PRINTF("vid based priority assign ok!\n");
				}
		#endif
		else{
			RTK_QOS_PRINTF("Priority assign failed!\n");
			return FAILED;
		}
	}
#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Dscpv6_Priority_Assign",strlen(_param[2])))))
	{
		if(rtk_dscpv6_based_priority_assign(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("ipv6 Dscp based priority assign ok!\n");
		}else{
			RTK_QOS_PRINTF("ipv6 Dscp based priority assign failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Dscpv6_Priority_Decision",strlen(_param[2])))))
	{
		if(rtk_dscpv6_priority_decision(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("ipv6 dscp based qos priority configure ok!\n");
		}else{
			RTK_QOS_PRINTF("ipv6 dscp based qos priority configure failed!\n");
			return FAILED;
		}
	}
#endif
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Queue_Num",strlen(_param[2])))))
	{
		if(rtk_queue_number(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Queue number assign ok!\n");
		}else{
			RTK_QOS_PRINTF("Queue number assign failed!\n");
			return FAILED;
		}
	}
#if defined(SET_QUEUE_FLOW_CONTROL)
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Queue_Flow_Control",strlen(_param[2])))))
	{
		if(rtk_queue_flow_control(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Queue Flow Control assign ok!\n");
		}else{
			RTK_QOS_PRINTF("Queue Flow Control failed!\n");
			return FAILED;
		}
	}
#endif
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Queue_Type",strlen(_param[2])))))
	{
		if(rtk_queue_type_strict(_num, _param, &send_data) > 0){
			strict_queue_show(send_data.qos_data.queue_type.queue);
		}else if(rtk_queue_type_weighted(_num, _param, &send_data) > 0){
			weighted_queue_show(send_data.qos_data.queue_type.queue);
		}else{
			RTK_QOS_PRINTF("Queue type assign failed\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Priority_to_Qid",strlen(_param[2])))))
	{
		if(rtk_priority_to_qid(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Priority mapping to qid ok!\n");
		}else{
			RTK_QOS_PRINTF("Priority mapping to qid failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"CPU_Priority_to_Qid",strlen(_param[2])))))
	{
		if(rtk_cpu_priority_to_qid(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("CPU Priority mapping to qid ok!\n");
		}else{
			RTK_QOS_PRINTF("CPU Priority mapping to qid failed!\n");
			return FAILED;
		}
	}
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Mark_to_Priority",strlen(_param[2])))))
	{
		if(rtk_mark_to_priority(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Mark mapping to priority ok!\n");
		}else{
			RTK_QOS_PRINTF("Mark mapping to priority failed!\n");
			return FAILED;
		}
	}
#endif
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Priority_Decision",strlen(_param[2])))))
	{
		if(rtk_priority_decision(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Priority decision configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Priority decision configure failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Remark",strlen(_param[2])))))
	{
		if((_param[3] != NULL) && (!(memcmp(_param[3], "DISABLE", strlen(_param[3]))))){
			if((_param[4] != NULL) && (!(memcmp(_param[4],"DSCP",strlen(_param[4]))))){
				send_data.action = DISABLE_DSCP_REMARK;
			}
			else if((_param[4] != NULL) && (!(memcmp(_param[4],"DOT1P",strlen(_param[4]))))){
				send_data.action = DISABLE_DOT1P_REMARK;
			}
			else{
				RTK_QOS_PRINTF("Remark disable failed!\n");
				return FAILED;
			}
		}
		else if(rtk_vlan_remark(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Vlan remark configure ok!\n");
		}else if(rtk_dscp_remark(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Dscp remark configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Remark configure failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Rate",strlen(_param[2])))))
	{
		if(rtk_queue_rate(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Queue rate configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Queue rate configure failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Port_Rate",strlen(_param[2])))))
	{
		if(rtk_port_rate(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Port rate configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Port rate configure failed!\n");
			return FAILED;
		}
	}
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"Flow_Control",strlen(_param[2])))))
	{
		if(rtk_flow_control_config(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Flow control configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Flow control configure failed!\n");
			return FAILED;
		}
	}
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"TC_COMMAND",strlen(_param[2])))))
	{
		if(rtk_tc_command_config(_num, _param, &send_data) > 0){
			RTK_QOS_PRINTF("Tc command configure ok!\n");
		}else{
			RTK_QOS_PRINTF("Tc command configure failed!\n");
			return FAILED;
		}
	}
#endif
	else if((_param[2] != NULL) && (!(memcmp(_param[2],"SHOW",strlen(_param[2])))))
	{
		if((_param[3] != NULL) && (!(memcmp(_param[3],"PORT_BASED_PRI",strlen(_param[3]))))){
			send_data.action = PORT_BASED_PRIORITY_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"VLAN_BASED_PRI",strlen(_param[3]))))){
			send_data.action = VLAN_BASED_PRIORITY_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"DSCP_BASED_PRI",strlen(_param[3]))))){
			send_data.action = DSCP_BASED_PRIORITY_SHOW;
		}
		#if defined(CONFIG_RTL_8197F)
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"VID_BASED_PRI",strlen(_param[3]))))){
			send_data.action = VID_BASED_PRIORITY_SHOW;
		}
		#endif
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"QUEUE_NUMBER",strlen(_param[3]))))){
			send_data.action = QUEUE_NUMBER_SHOW;
		}
	#if defined(SET_QUEUE_FLOW_CONTROL)
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"QUEUE_FLOW_CONTROL",strlen(_param[3]))))){
			send_data.action = QUEUE_FLOW_CONTROL_SHOW;
		}
	#endif
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"QUEUE_TYPE_STRICT",strlen(_param[3]))))){
			send_data.action = QUEUE_TYPE_STRICT_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"QUEUE_TYPE_WEIGHTED",strlen(_param[3]))))){
			send_data.action = QUEUE_TYPE_WEIGHTED_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"PRIORITY_TO_QID",strlen(_param[3]))))){
			send_data.action = PRIORITY_TO_QID_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"PRIORITY_DECISION",strlen(_param[3]))))){
			send_data.action = PRIORITY_DECISION_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"CPU_PRIORITY_TO_QID",strlen(_param[3]))))){
			send_data.action = CPU_PRIORITY_TO_QID_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"VLAN_REMARK",strlen(_param[3]))))){
			send_data.action = VLAN_REMARK_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"DSCP_REMARK",strlen(_param[3]))))){
			send_data.action = DSCP_REMARK_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"QUEUE_RATE",strlen(_param[3]))))){
			send_data.action = QUEUE_RATE_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"PORT_RATE",strlen(_param[3]))))){
			send_data.action = PORT_RATE_SHOW;
		}else if((_param[3] != NULL) && (!(memcmp(_param[3],"FLOW_CONTROL_CONFIG",strlen(_param[3]))))){
			send_data.action = FLOW_CONTROL_CONFIGURATION_SHOW;
		}
		#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"DSCPV6_BASED_PRI",strlen(_param[3]))))){
			send_data.action = DSCPV6_BASED_PRIORITY_SHOW;
		}
		else if((_param[3] != NULL) && (!(memcmp(_param[3],"DSCPV6_PRI_DECISION",strlen(_param[3]))))){
			send_data.action = DSCPV6_PRIORITY_DECISION_SHOW;
		}
		#endif
		else{
			RTK_QOS_PRINTF("rtk qos show failed!\n");
			return FAILED;
		}
	}
	else
	{
		RTK_QOS_PRINTF("This command is not formal rtk qos cmd, please try again!\n");
		return FAILED;
	}

	send_info.len = sizeof(struct qos_cmd_info_s);
	send_info.data = &send_data;
	recv_info.len= sizeof(struct qos_cmd_info_s);
	recv_info.data=&recv_data;
	rtk_netlink(NETLINK_RTK_HW_QOS,&send_info, &recv_info);

	return SUCCESS;
}


