using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace WS_SB.DB
{
    public enum SnsType
    {
        Local = 1,
        Google = 2
    }

    [Table("Registration")]
    public class RegisterDB
    {
        [Column("Id")]
        public int                          RegisterDBId { get; set; }

        [MaxLength(40)]
        public string                       Name { get; set; }

        [MaxLength(40)]
        public string                       Password { get; set; }

        public AccountDB                    Account { get; set; }
    }

    [Table("Account")]
    public class AccountDB
    {
        [Column("Id")]
        public int                          AccountDBId { get; set; }

        [Column("SnsType")]
        public SnsType                      AccountSnsType { get; set; }

        [Column("SnsSub")]
        public string                       AccountSnsSub { get; set; }

        public int?                         RegisterDBId { get; set; }

        public RegisterDB?                  Register { get; set; }

        public virtual ICollection<UserDB>  Users { get; set; }
    }

    [Table("User")]
    public class UserDB
    {
        [Column("Id")]
        public int                          UserDBId { get; set; }

        [MaxLength(40)]
        public string                       UserName { get; set; }

        public int                          CostumeSetting { get; set; } = 0;

        public int                          AccountDBId { get; set; }

        public virtual RankingDB            Ranking { get; set; }
    }

    [Table("Ranking")]
    public class RankingDB
    {
        [Column("Id")]
        public int                          RankingDBId { get; set; }

        public int                          Score { get; set; } = 0;

        public virtual UserDB               User { get; set; }

        public int                          UserDBId { get; set; }
    }
}
