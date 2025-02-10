using WS_SB.DB;

namespace WS_SB
{
    public static class Extentions
    {
        public static bool SaveChangesEx(this ApplicationDbContext db)
        {
            try
            {
                db.SaveChanges();
                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
