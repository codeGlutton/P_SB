using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;
using Microsoft.IdentityModel.Tokens;

namespace WS_SB.DB
{
    public class ApplicationDbContext : DbContext
    {
        public DbSet<AccountDB>     Accounts { get; set; }
        public DbSet<RegisterDB>    Registers { get; set; }

        public ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
            : base(options)
        {
        }

        // 테이블 상세 설정
        protected override void OnModelCreating(ModelBuilder builder)
        {
            builder.Entity<RegisterDB>(
                nestedBuilder =>
                {
                    nestedBuilder
                        .HasIndex(model => model.Name)
                        .IsUnique();
                    nestedBuilder
                        .HasOne(model => model.Account)
                        .WithOne(child => child.Register)
                        .HasForeignKey<AccountDB>(child => child.RegisterDBId)
                        .IsRequired(false)
                        .OnDelete(DeleteBehavior.Cascade);
                }
            );

            builder.Entity<AccountDB>(
                nestedBuilder =>
                {
                    nestedBuilder
                        .HasIndex(model => new { model.AccountSnsType, model.AccountSnsSub })
                        .IsUnique();
                    nestedBuilder
                        .HasMany(model => model.Users)
                        .WithOne()
                        .HasForeignKey(child => child.AccountDBId);
                }
            );

            builder.Entity<UserDB>()
                .HasOne(model => model.Ranking)
                .WithOne(child => child.User)
                .HasForeignKey<RankingDB>(child => child.UserDBId)
                .IsRequired();

            builder.Entity<RankingDB>()
                .HasIndex(model => model.Score);
        }
    }
}
