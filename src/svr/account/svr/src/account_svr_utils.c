#include "gd/app/app_log.h"
#include "account_svr_ops.h"

int account_svr_validate_passwd(
    account_svr_t svr, SVR_ACCOUNT_BASIC const * account,
    SVR_ACCOUNT_LOGIC_ID const * logic_id, const char * passwd)
{
    switch(logic_id->account_type) {
    case SVR_ACCOUNT_MAC:
        return 0;
    default:
        if (account->password[0] == 0) {
            APP_CTX_ERROR(svr->m_app, "%s: validate_passwd: no password!", account_svr_name(svr));
            return SVR_ACCOUNT_ERROR_NO_PASSWD;
        }

        if (strcmp(passwd, account->password) != 0) {
            APP_CTX_ERROR(svr->m_app, "%s: validate_passwd: password mismatch!", account_svr_name(svr));
            return SVR_ACCOUNT_ERROR_PASSWD;
        }
        return 0;
    }
}
