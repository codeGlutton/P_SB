using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace WS_SB.Migrations
{
    /// <inheritdoc />
    public partial class AddPlayerAssetTableIdField : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.AddColumn<int>(
                name: "AssetTableId",
                table: "Player",
                type: "int",
                nullable: false,
                defaultValue: 0);
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropColumn(
                name: "AssetTableId",
                table: "Player");
        }
    }
}
