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
        public int                              RegisterDBId { get; set; }

        [MaxLength(40)]
        public string                           Name { get; set; }

        [MaxLength(40)]
        public string                           Password { get; set; }

        public AccountDB                        Account { get; set; }
    }

    [Table("Account")]
    public class AccountDB
    {
        [Column("Id")]
        public int                              AccountDBId { get; set; }

        [Column("SnsType")]
        public SnsType                          AccountSnsType { get; set; }

        [Column("SnsSub")]
        public string                           AccountSnsSub { get; set; }

        public int?                             RegisterDBId { get; set; }

        public RegisterDB?                      Register { get; set; }

        public virtual ICollection<PlayerDB>    Players { get; set; }
    }

    [Table("Player")]
    public class PlayerDB
    {
        [Column("Id")]
        public int                              PlayerDBId { get; set; }

        public int                              AssetTableId { get; set; } = 0;

        [MaxLength(40)]
        public string?                          Name { get; set; }

        [DataType(DataType.Date)]
        [Column(TypeName = "date")]
        public DateTime                         CreatedTime { get; set; } = DateTime.UtcNow;

        public int                              CostumeSetting { get; set; } = 0;

        public int                              Score { get; set; } = 0;

        public int                              AccountDBId { get; set; }
    }
}
