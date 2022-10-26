#pragma once

#define FOR_EACH_LIST(ite, head)			for (ite=(head)->Flink; ite!=(head) && (ite); ite=(ite)->Flink)
#define FOR_EACH_LIST_SAFE(ite, temp, head)	for (ite=(head)->Flink, temp=(ite)->Flink; ite!=(head) && (ite); ite=(temp), temp=(temp)->Flink)