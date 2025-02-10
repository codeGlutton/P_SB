using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace WS_SB.DB
{
    [Table("Account")]
    public class AccountDB
    {
        [Column("Id")]
        public int AccountDBId { get; set; }
        public string AccountName { get; set; }
        public string Password { get; set; }
    }
}
