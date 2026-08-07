/*****************************************************************/ /**
* @file dmhttp_demo.c
* @brief dmhttp demo implementation
* @author kruskal.zhu@quectel.com
* @date 2025-09
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date    <th>Version <th>Author      <th>Description
* <tr><td>2025-10 <td>1.0     <td>kruskal.zhu <td>Initial version
* </table>
**********************************************************************/

#include "qosa_sys.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include "qcm_dmhttp.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG                 LOG_TAG

#define DMHTTP_DEMO_TASK_STACK_SIZE 4096

static qosa_task_t g_dm_task = QOSA_NULL;
static qosa_sem_t  g_dm_sem = QOSA_NULL;

static void unir_dmhttp_cfg(void)
{
    // RESUME_SAVE
    {
        int resume_save = 0;

        // First, get whether to turn on the power-down save function
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_RESUME_SAVE, &resume_save);
        QLOGD("resume_save: %d", resume_save);
        if (QCM_DMHTTP_CFG_RESUME_SAVE_ENABLE == resume_save)
        {
            // Enable the power-down save function, and configure the parameters according to the module before the last shutdown
            return;
        }

        // Set the power-down save function
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_RESUME_SAVE, QCM_DMHTTP_CFG_RESUME_SAVE_ENABLE);
    }

    // EP_INFO
    {
        char *data = QOSA_NULL;

        // Fixed 12 EP_INFO messages:
        // brand; model; appKey; sdkVersion; apiVersion; apiType; templateId; imei1; imei2; preField3; preField1; preField2;
        // EP_INFO Users can directly use the default values of the module presets without configuration, and a setup demonstration is provided here
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_EP_INFO, "brand", "Quectel");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_EP_INFO, "appKey", "M100000089");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_EP_INFO, "apiVersion", "4.0.1");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_EP_INFO, "apiType", "I");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_EP_INFO, "templateId", "TY000035");
        // If the authentication request fails, the IMEI may have expired, and the platform will add the IMEI number to the blacklist, and you can change the unused IMEI by yourself

        // Query EP_INFO a configuration:
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_EP_INFO, "brand", &data);
        if (data != QOSA_NULL)
        {
            QLOGD("brand: %s", data);
            data = QOSA_NULL;
        }

        // Once configured, EP_INFO can preview the contents of the registration form query request body
        data = qcm_dmhttp_ep_info_stitch_func();
        if (data != QOSA_NULL)
        {
            QLOGD("ep_info: %s", data);
            // qcm_dmhttp_ep_info_stitch_func Memory needs to be manually freed after execution
            qosa_free(data);
        }
    }

    // FIELDCFG
    {
        char *data = QOSA_NULL;

        // A new FIELDCFG configuration has been added, which allows users to configure their own fields based on the fields contained in the actual FIELDCFG of the server
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_ADD, "platform", "platform");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_ADD, "createTime", "createTime");
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_ADD, "ram", "ram");

        // Once FIELDCFG is configured, you can preview the annotated registration request message
        data = qcm_dmhttp_fieldcfg_stitch_func(QOSA_FALSE);
        if (data != QOSA_NULL)
        {
            QLOGD("FIELDCFG: %s", data);
            // qcm_dmhttp_fieldcfg_stitch_func Memory needs to be manually freed after execution
            qosa_free(data);
        }

        // Delete one of the specified FIELDCFG configurations
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_DEL, "ram");

        // Delete all FIELDCFG configurations
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_DEL, QOSA_NULL);

        // A new FIELDCFG configuration has been added
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_FIELDCFG, QCM_DMHTTP_CFG_FIELDCFG_ADD, "createTime", "createTime");
    }

    // SIM
    {
        int sim = 0;  // Generally, the default card is 0

        // set SIM ID
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_SIM, sim);

        // get SIM ID
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_SIM, &sim);
        QLOGD("sim: %d", sim);
    }

    // CID
    {
        int cid = 1;

        // set PDP ID
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_CID, cid);

        // get PDP ID
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_CID, &cid);
        QLOGD("cid: %d", cid);
    }

    // KEY
    {
        char *key = QOSA_NULL;

        // Set the Report device information encryption key
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_KEY, "95GozTUi33nUK05ha526100QFbk0Y38M");

        // Obtain the encryption key for reporting device information
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_KEY, &key);
        QLOGD("key: %s", key);
    }

    // HEART_BEAT
    {
        int           hb_enable = QCM_DMHTTP_CFG_HB_ENABLE;  // Whether to enable the heartbeat function
        qosa_uint32_t hb_time = 5;  // After the previous registration/heartbeat request is successful, the next heartbeat interval time
        qosa_uint32_t hb_cnt = 4;   // Heart beats

        // set HEART_BEAT
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_HEART_BEAT, hb_enable, hb_time, hb_cnt);

        // get HEART_BEAT
        qcm_dmhttp_cfg_get(QCM_DMHTTP_CFG_HEART_BEAT, &hb_enable, &hb_time, &hb_cnt);
        QLOGD("hb_enable: %d, hb_time: %d, hb_cnt: %d", hb_enable, hb_time, hb_cnt);
    }
}

void unir_dmhttp_notify_cb(qcm_dmhttp_type_e notify_type, int result, int http_code, qosa_uint32_t hb_remain_cnt, void *cb_data)
{
    char *download_data = QOSA_NULL;

    QOSA_UNUSED(cb_data);

    QLOGD("notify_type: %d", notify_type);
    QLOGD("result: %d", result);
    QLOGD("http_code: %d", http_code);
    QLOGD("hb_remain_cnt: %d", hb_remain_cnt);

    // Get the reply message returned by the DMHTTP server
    download_data = qcm_dmhttp_get_download_data(notify_type);
    if (download_data != QOSA_NULL)
    {
        QLOGD("download_data: %s", download_data);
    }

    if (QCM_DMHTTP_GET_INFO == notify_type || (QCM_DMHTTP_POST_REG == notify_type && (result != 0 || http_code != 200)) || (0 == result && 0 == hb_remain_cnt))
    {
        // If it's a GET_INFO notification type
        // or POST_REG failed the first registration request
        // or POST_REG, HEART_BEAT the last DMHTTP execution was successful
        QLOGD("sem release");
        qosa_sem_release(g_dm_sem);
    }
}

static void unir_dmhttp_demo_task(void *ctx)
{
    int ret = 0;

    // Delayed 15s start
    qosa_task_sleep_sec(15);

    // Set up the configuration items
    unir_dmhttp_cfg();

    // After the registration qcm_dmhttp_perform is executed, the DMHTTP notification callback of the business results is callbacked
    qcm_dmhttp_set_notify_cb(unir_dmhttp_notify_cb, QOSA_NULL);

    // Initiate a registration form acquisition request
    {
        QLOGD("GET_INFO");

        // Set the URL that initiates the Get registration form request
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_URL, QCM_DMHTTP_URL_GET_INFO);

        // Only one business task is allowed at a time, and the next task request can be executed until the current business execution is completed
        ret = qcm_dmhttp_perform(QCM_DMHTTP_GET_INFO);
        if (ret != QCM_DMHTTP_ERR_OK)
        {
            goto exit;
        }

        // Wait for the notification callback unir_dmhttp_notify_cb release semaphore
        qosa_sem_wait(g_dm_sem, QOSA_WAIT_FOREVER);
    }

    // Originated from Registration Request. If you enable self-registration of heartbeats, the heartbeat request will be automatically initiated after the registration request is completed
    {
        QLOGD("POST_REG");

        // Set the URL that initiates the Get registration form request
        qcm_dmhttp_cfg_set(QCM_DMHTTP_CFG_URL, QCM_DMHTTP_URL_POST_REG);

        // Only one business task is allowed at a time, and the next task request can be executed until the current business execution is completed
        qcm_dmhttp_perform(QCM_DMHTTP_POST_REG);
        if (ret != QCM_DMHTTP_ERR_OK)
        {
            goto exit;
        }

        // Wait for the notification callback unir_dmhttp_notify_cb release semaphore
        qosa_sem_wait(g_dm_sem, QOSA_WAIT_FOREVER);
    }

exit:
    qosa_sem_delete(g_dm_sem);
    g_dm_sem = QOSA_NULL;

    g_dm_task = QOSA_NULL;
}

void unir_dmhttp_demo_init(void)
{
    int err = 0;

    err = qosa_sem_create_ex(&g_dm_sem, 0, 1);
    if (err != QOSA_OK)
    {
        QLOGD("task create error");
        return;
    }

    err = qosa_task_create(&g_dm_task, DMHTTP_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL, "dm_demo", unir_dmhttp_demo_task, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGD("task create error");
        return;
    }
}
UNIRTOS_APP_EXPORT(326, "dmhttp_demo", unir_dmhttp_demo_init);