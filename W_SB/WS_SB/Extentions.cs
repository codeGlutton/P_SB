using Microsoft.Data.SqlClient;
using Microsoft.EntityFrameworkCore;
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
            catch (DbUpdateConcurrencyException ex)
            {
                ex.Entries.Single().Reload();
                return false;
            }
            catch
            {
                throw;
            }
        }
    }
}
