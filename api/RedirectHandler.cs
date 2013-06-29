namespace WixToolset.Web.Api
{
    using System;
    using System.Web;
    using System.Web.Configuration;

    public class RedirectHandler : IHttpHandler
    {
        public bool IsReusable
        {
            get { return true; }
        }

        public void ProcessRequest(HttpContext context)
        {
            string key = "redirect." + context.Request.Url.AbsolutePath.Trim('/').ToLowerInvariant();
            string redirect = WebConfigurationManager.AppSettings[key] ?? "/";

            context.Response.Redirect(redirect, true);
        }
    }
}
